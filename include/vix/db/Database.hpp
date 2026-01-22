/**
 *
 *  @file Database.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_DB_DATABASE_HPP
#define VIX_DB_DATABASE_HPP

#include <string>
#include <vix/db/pool/ConnectionPool.hpp>

namespace vix::config
{
  class Config;
}

namespace vix::db
{
  enum class Engine
  {
    MySQL,
    SQLite
  };

  struct MySQLConfig
  {
    std::string host;
    std::string user;
    std::string password;
    std::string database;
    PoolConfig pool{};
  };

  struct SQLiteConfig
  {
    std::string path;
    PoolConfig pool{};
  };

  struct DbConfig
  {
    Engine engine{Engine::MySQL};
    MySQLConfig mysql{};
    SQLiteConfig sqlite{};
  };

  DbConfig make_db_config_from_vix_config(const vix::config::Config &cfg);

  class Database
  {
  public:
    explicit Database(const DbConfig &cfg);

    Engine engine() const noexcept { return cfg_.engine; }
    const DbConfig &config() const noexcept { return cfg_; }

    ConnectionPool &pool() noexcept { return pool_; }
    const ConnectionPool &pool() const noexcept { return pool_; }

  private:
    DbConfig cfg_;
    ConnectionPool pool_;
  };
} // namespace vix::db

#endif
