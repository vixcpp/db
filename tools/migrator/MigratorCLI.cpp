/**
 *
 *  @file MigratorCLI.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#include "MigratorCLI.hpp"

#include <vix/db/db.hpp>
#include <vix/db/mig/FileMigrationsRunner.hpp>
#include <vix/db/core/Drivers.hpp>

#if VIX_DB_HAS_MYSQL
#include <vix/db/drivers/mysql/MySQLDriver.hpp>
#endif

#if VIX_DB_HAS_SQLITE
#include <vix/db/drivers/sqlite/SQLiteDriver.hpp>
#endif

#include "MakeMigrations.hpp"

#include <iostream>
#include <stdexcept>
#include <memory>
#include <filesystem>
#include <vector>
#include <string>
#include <optional>

namespace vix::db::tools
{
  void MigratorCLI::printUsage(const char *prog)
  {
    std::cout
        << "Vix ORM Migrator\n\n"
        << "Usage:\n"
        << "  " << prog << " <host> <user> <pass> <db> migrate   [--dir <migrations_dir>]\n"
        << "  " << prog << " <host> <user> <pass> <db> rollback  --steps <n> [--dir <migrations_dir>]\n"
        << "  " << prog << " <host> <user> <pass> <db> status    [--dir <migrations_dir>]\n\n"
        << "  " << prog << " makemigrations --new <schema.json> [--snapshot <schema.json>] "
                           "[--dir <migrations_dir>] [--name <label>] [--dialect mysql|sqlite]\n\n"
        << "Examples:\n"
        << "  " << prog << " tcp://127.0.0.1:3306 root '' mydb migrate --dir ./migrations\n"
        << "  " << prog << " tcp://127.0.0.1:3306 root '' mydb rollback --steps 1\n"
        << "  " << prog << " tcp://127.0.0.1:3306 root '' mydb status --dir db/migrations\n"
        << "  " << prog << " makemigrations --new ./schema.new.json --snapshot ./schema.json --dir ./migrations --name create_users\n";
  }

  static bool has_flag(const std::vector<std::string> &args, const std::string &key)
  {
    for (const auto &a : args)
      if (a == key)
        return true;
    return false;
  }

  static std::string get_flag_value(const std::vector<std::string> &args, const std::string &key)
  {
    for (size_t i = 0; i + 1 < args.size(); ++i)
      if (args[i] == key)
        return args[i + 1];
    return {};
  }

  static std::string get_flag_value_eq(const std::vector<std::string> &args, const std::string &prefix)
  {
    for (const auto &a : args)
      if (a.rfind(prefix, 0) == 0)
        return a.substr(prefix.size());
    return {};
  }

  static std::string parse_opt(const std::vector<std::string> &args, const std::string &key)
  {
    std::string v = get_flag_value(args, key);
    if (!v.empty())
      return v;
    return get_flag_value_eq(args, key + "=");
  }

  std::string MigratorCLI::getFlagValue(const std::vector<std::string> &args, const std::string &key)
  {
    return get_flag_value(args, key);
  }

  bool MigratorCLI::hasFlag(const std::vector<std::string> &args, const std::string &key)
  {
    return has_flag(args, key);
  }

  std::string MigratorCLI::parseDir(const std::vector<std::string> &args)
  {
    std::string v = parse_opt(args, "--dir");
    if (!v.empty())
      return v;
    return "migrations";
  }

  int MigratorCLI::parseStepsOrThrow(const std::vector<std::string> &args)
  {
    std::string steps_s = parse_opt(args, "--steps");
    if (steps_s.empty())
      throw std::runtime_error("rollback requires --steps <n>");

    int steps = 0;
    try
    {
      steps = std::stoi(steps_s);
    }
    catch (...)
    {
      throw std::runtime_error("invalid --steps value (must be an integer)");
    }

    if (steps <= 0)
      throw std::runtime_error("--steps must be >= 1");

    return steps;
  }

  void MigratorCLI::validateOrThrow(const Options &opt)
  {
    if (opt.help)
      return;

    if (opt.command == "makemigrations")
    {
      if (opt.newSchemaPath.empty())
        throw std::runtime_error("makemigrations requires --new <schema.json>");
      if (opt.migrationsDir.empty())
        throw std::runtime_error("migrations dir is empty (use --dir <path>)");

      if (opt.dialect != "mysql" && opt.dialect != "sqlite")
        throw std::runtime_error("invalid --dialect (use mysql or sqlite)");

      return;
    }

    // DB commands
    if (opt.host.empty() || opt.user.empty() || opt.db.empty())
      throw std::runtime_error("missing required args: <host> <user> <pass> <db> <command>");

    if (opt.command != "migrate" && opt.command != "rollback" && opt.command != "status")
      throw std::runtime_error("unknown command: " + opt.command);

    if (opt.command == "rollback" && opt.steps <= 0)
      throw std::runtime_error("rollback requires --steps <n>");

    if (opt.migrationsDir.empty())
      throw std::runtime_error("migrations dir is empty (use --dir <path>)");
  }

  MigratorCLI::Options MigratorCLI::parseArgsOrThrow(int argc, char **argv)
  {
    Options opt;

    std::vector<std::string> all;
    all.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i)
      all.emplace_back(argv[i]);

    if (argc <= 1 || has_flag(all, "-h") || has_flag(all, "--help"))
    {
      opt.help = true;
      return opt;
    }

    // Special-case: makemigrations (no DB args)
    if (argc >= 2 && std::string(argv[1]) == "makemigrations")
    {
      opt.command = "makemigrations";

      std::vector<std::string> extra;
      for (int i = 2; i < argc; ++i)
        extra.emplace_back(argv[i]);

      opt.newSchemaPath = parse_opt(extra, "--new");
      opt.snapshotPath = parse_opt(extra, "--snapshot");
      opt.name = parse_opt(extra, "--name");
      opt.dialect = parse_opt(extra, "--dialect");
      opt.migrationsDir = parseDir(extra);

      // defaults
      if (opt.snapshotPath.empty())
        opt.snapshotPath = "schema.json";
      if (opt.name.empty())
        opt.name = "auto";
      if (opt.dialect.empty())
        opt.dialect = "mysql";

      validateOrThrow(opt);
      return opt;
    }

    // DB commands require: <host> <user> <pass> <db> <command>
    if (argc < 6)
      throw std::runtime_error("not enough arguments");

    opt.host = argv[1];
    opt.user = argv[2];
    opt.pass = argv[3];
    opt.db = argv[4];
    opt.command = argv[5];

    std::vector<std::string> extra;
    for (int i = 6; i < argc; ++i)
      extra.emplace_back(argv[i]);

    opt.migrationsDir = parseDir(extra);

    if (opt.command == "rollback")
      opt.steps = parseStepsOrThrow(extra);

    validateOrThrow(opt);
    return opt;
  }

  int MigratorCLI::run(int argc, char **argv)
  {
    try
    {
      Options opt = parseArgsOrThrow(argc, argv);

      if (opt.help)
      {
        printUsage(argv[0]);
        return 0;
      }

      // makemigrations does not require a DB connection
      if (opt.command == "makemigrations")
      {
        return run_make_migrations(opt);
      }

      vix::db::ConnectionPtr conn;
      std::unique_ptr<vix::db::FileMigrationsRunner> runner;

#if VIX_DB_HAS_MYSQL
      {
        // Declared in MySQLDriver.hpp, implemented in MySQLDriver.cpp
        auto factory = vix::db::make_mysql_factory(opt.host, opt.user, opt.pass, opt.db);
        conn = factory();
        runner = std::make_unique<vix::db::FileMigrationsRunner>(*conn, opt.migrationsDir);
      }
#elif VIX_DB_HAS_SQLITE
      {
        // Declared in SQLiteDriver.hpp, implemented in SQLiteDriver.cpp
        auto factory = vix::db::make_sqlite_factory(opt.db);
        conn = factory();
        runner = std::make_unique<vix::db::FileMigrationsRunner>(*conn, opt.migrationsDir);
      }
#else
      std::cerr << "[ERR] vix_db_migrator built without DB drivers.\n"
                << "Enable one with:\n"
                << "  -DVIX_DB_USE_MYSQL=ON (and rebuild)\n"
                << "  -DVIX_DB_USE_SQLITE=ON (and rebuild)\n";
      return 1;
#endif

      if (opt.command == "migrate")
      {
        runner->applyAll();
        std::cout << "[OK] migrations applied\n";
        return 0;
      }

      if (opt.command == "rollback")
      {
        runner->rollback(opt.steps);
        std::cout << "[OK] rollback " << opt.steps << " step(s)\n";
        return 0;
      }

      if (opt.command == "status")
      {
        std::cout << "[OK] migrations dir: " << opt.migrationsDir << "\n";
        std::cout << "Tip: implement FileMigrationsRunner::status() to show applied vs pending.\n";
        return 0;
      }

      printUsage(argv[0]);
      return 1;
    }
    catch (const std::exception &e)
    {
      std::cerr << "[ERR] " << e.what() << "\n";
      std::cerr << "Tip: run with --help\n";
      return 1;
    }
  }

} // namespace vix::db::tools
