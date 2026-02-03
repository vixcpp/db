/**
 *
 *  @file Database.hpp
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
  /**
   * @brief Supported database engines.
   */
  enum class Engine
  {
    /// MySQL-compatible database engine
    MySQL,

    /// SQLite embedded database engine
    SQLite
  };

  /**
   * @brief Configuration parameters for a MySQL database.
   */
  struct MySQLConfig
  {
    /// Database host
    std::string host;

    /// Username
    std::string user;

    /// Password
    std::string password;

    /// Database name
    std::string database;

    /// Connection pool configuration
    PoolConfig pool{};
  };

  /**
   * @brief Configuration parameters for a SQLite database.
   */
  struct SQLiteConfig
  {
    /// Path to the SQLite database file
    std::string path;

    /// Connection pool configuration
    PoolConfig pool{};
  };

  /**
   * @brief Unified database configuration.
   *
   * Holds engine-specific configuration while exposing a single
   * entry point for database initialization.
   */
  struct DbConfig
  {
    /// Selected database engine
    Engine engine{Engine::MySQL};

    /// MySQL-specific configuration
    MySQLConfig mysql{};

    /// SQLite-specific configuration
    SQLiteConfig sqlite{};
  };

  /**
   * @brief Build a database configuration from a Vix configuration.
   *
   * Extracts database-related settings from a vix::config::Config
   * instance and maps them to a DbConfig structure.
   *
   * @param cfg Vix configuration object.
   * @return Parsed database configuration.
   */
  DbConfig make_db_config_from_vix_config(const vix::config::Config &cfg);

  /**
   * @brief High-level database facade.
   *
   * Database owns the connection pool and exposes access to it,
   * providing a unified entry point for database access within
   * the application.
   *
   * Engine selection and driver wiring are performed at construction
   * time based on the provided DbConfig.
   */
  class Database
  {
  public:
    /**
     * @brief Construct a database instance.
     *
     * Initializes the underlying connection pool according to
     * the selected engine and configuration.
     *
     * @param cfg Database configuration.
     */
    explicit Database(const DbConfig &cfg);

    /**
     * @brief Return the selected database engine.
     *
     * @return Database engine.
     */
    Engine engine() const noexcept { return cfg_.engine; }

    /**
     * @brief Access the database configuration.
     *
     * @return Database configuration.
     */
    const DbConfig &config() const noexcept { return cfg_; }

    /**
     * @brief Access the connection pool.
     *
     * @return Reference to the connection pool.
     */
    ConnectionPool &pool() noexcept { return pool_; }

    /**
     * @brief Access the connection pool (const).
     *
     * @return Const reference to the connection pool.
     */
    const ConnectionPool &pool() const noexcept { return pool_; }

  private:
    DbConfig cfg_;
    ConnectionPool pool_;
  };

} // namespace vix::db

#endif // VIX_DB_DATABASE_HPP
