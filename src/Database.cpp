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

#include <memory>
#include <stdexcept>
#include <utility>

namespace vix::db
{
  DbConfig make_db_config_from_vix_config(const vix::config::Config &cfg)
  {
    DbConfig out;

    const auto engineStr = cfg.getString("db.engine", "sqlite");
    if (engineStr == "mysql")
    {
      out.engine = Engine::MySQL;
    }
    else
    {
      out.engine = Engine::SQLite;
    }

    out.mysql.host = cfg.getString("db.host", "tcp://127.0.0.1:3306");
    out.mysql.user = cfg.getString("db.user", "root");
    out.mysql.password = cfg.getString("db.password", "");
    out.mysql.database = cfg.getString("db.database", "");

    out.mysql.pool.min =
        static_cast<std::size_t>(cfg.getInt("db.pool.min", 1));
    out.mysql.pool.max =
        static_cast<std::size_t>(cfg.getInt("db.pool.max", 8));

    out.sqlite.path = cfg.getString("db.sqlite", "vix.db");
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
