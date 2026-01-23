/**
 *
 *  @file MigratorCLI.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_DB_MIGRATOR_CLI_HPP
#define VIX_DB_MIGRATOR_CLI_HPP

#include <string>
#include <vector>

namespace vix::db::tools
{
  class MigratorCLI
  {
  public:
    struct Options
    {
      std::string host;
      std::string user;
      std::string pass;
      std::string db;

      std::string command; // migrate | rollback | status | makemigrations
      std::string migrationsDir = "migrations";

      int steps = 0; // rollback steps
      bool help = false;

      // makemigrations options
      std::string snapshotPath = "schema.json"; // schema snapshot (old -> updated)
      std::string newSchemaPath;                // required: schema.new.json
      std::string name = "auto";                // label in filename
      std::string dialect = "mysql";            // mysql | sqlite (mysql only for now)
    };

    static int run(int argc, char **argv);

  private:
    static void printUsage(const char *prog);

    static Options parseArgsOrThrow(int argc, char **argv);

    static std::string getFlagValue(
        const std::vector<std::string> &args,
        const std::string &key);

    static bool hasFlag(const std::vector<std::string> &args,
                        const std::string &key);

    static std::string parseDir(const std::vector<std::string> &args);

    static int parseStepsOrThrow(const std::vector<std::string> &args);

    static void validateOrThrow(const Options &opt);
  };
} // namespace vix::db::tools

#endif
