/**
 *
 *  @file Database.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#include <vix/db/Database.hpp>

#if VIX_DB_HAS_MYSQL
#include <vix/db/drivers/mysql/MySQLDriver.hpp>
#endif

#if VIX_DB_HAS_SQLITE
#include <vix/db/drivers/sqlite/SQLiteDriver.hpp>
#endif

#include <vix/config/Config.hpp>

#include <algorithm>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace vix::db
{
  namespace
  {
    [[nodiscard]] std::string to_lower_ascii(std::string value)
    {
      std::transform(
          value.begin(),
          value.end(),
          value.begin(),
          [](unsigned char c)
          { return static_cast<char>(std::tolower(c)); });

      return value;
    }

    [[nodiscard]] std::string build_mysql_host_string(
        const std::string &host,
        int port)
    {
      return "tcp://" + host + ":" + std::to_string(port);
    }
  } // namespace

  DbConfig make_db_config_from_vix_config(const vix::config::Config &cfg)
  {
    DbConfig out;

    const std::string engine_str =
        to_lower_ascii(cfg.getString("database.engine", "sqlite"));

    if (engine_str == "mysql")
    {
      out.engine = Engine::MySQL;
    }
    else
    {
      out.engine = Engine::SQLite;
    }

    const std::string mysql_host =
        cfg.getString("database.default.host", "127.0.0.1");
    const int mysql_port =
        cfg.getInt("database.default.port", 3306);

    out.mysql.host = build_mysql_host_string(mysql_host, mysql_port);
    out.mysql.user = cfg.getString("database.default.user", "root");
    out.mysql.password = cfg.getDbPasswordFromEnv();
    out.mysql.database = cfg.getString("database.default.name", "");

    out.mysql.pool.min = static_cast<std::size_t>(
        std::max(1, cfg.getInt("database.pool.min", 1)));

    out.mysql.pool.max = static_cast<std::size_t>(
        std::max(static_cast<int>(out.mysql.pool.min),
                 cfg.getInt("database.pool.max", 8)));

    out.sqlite.path = cfg.getString("database.sqlite.path", "vix.db");
    out.sqlite.pool.min = out.mysql.pool.min;
    out.sqlite.pool.max = out.mysql.pool.max;

    return out;
  }

  namespace
  {
    ConnectionFactory makeFactoryFor(const DbConfig &cfg)
    {
      switch (cfg.engine)
      {
      case Engine::MySQL:
      {
#if VIX_DB_HAS_MYSQL
        return make_mysql_factory(
            cfg.mysql.host,
            cfg.mysql.user,
            cfg.mysql.password,
            cfg.mysql.database);
#else
        throw std::runtime_error(
            "MySQL requested but VIX_DB_HAS_MYSQL=0");
#endif
      }

      case Engine::SQLite:
      {
#if VIX_DB_HAS_SQLITE
        return make_sqlite_factory(cfg.sqlite.path);
#else
        throw std::runtime_error(
            "SQLite requested but VIX_DB_HAS_SQLITE=0");
#endif
      }

      default:
        throw std::runtime_error("Unsupported database engine");
      }
    }

    PoolConfig poolConfigFor(const DbConfig &cfg)
    {
      switch (cfg.engine)
      {
      case Engine::MySQL:
        return cfg.mysql.pool;
      case Engine::SQLite:
        return cfg.sqlite.pool;
      default:
        return {};
      }
    }

    std::shared_ptr<ConnectionPool> makePoolFor(const DbConfig &cfg)
    {
      auto pool = std::make_shared<ConnectionPool>(
          makeFactoryFor(cfg),
          poolConfigFor(cfg));

      pool->warmup();
      return pool;
    }
  } // namespace

  Database::Database(const DbConfig &cfg)
      : cfg_(cfg), pool_(makePoolFor(cfg))
  {
  }

  Database Database::mysql(std::string host,
                           std::string user,
                           std::string password,
                           std::string database,
                           PoolConfig pool)
  {
    DbConfig cfg;
    cfg.engine = Engine::MySQL;
    cfg.mysql.host = std::move(host);
    cfg.mysql.user = std::move(user);
    cfg.mysql.password = std::move(password);
    cfg.mysql.database = std::move(database);
    cfg.mysql.pool = pool;
    return Database(cfg);
  }

  Database Database::sqlite(std::string path,
                            PoolConfig pool)
  {
    DbConfig cfg;
    cfg.engine = Engine::SQLite;
    cfg.sqlite.path = std::move(path);
    cfg.sqlite.pool = pool;
    return Database(cfg);
  }

} // namespace vix::db
