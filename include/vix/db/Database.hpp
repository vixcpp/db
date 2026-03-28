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

#include <memory>
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
    MySQL,
    SQLite
  };

  /**
   * @brief Configuration parameters for a MySQL database.
   */
  struct MySQLConfig
  {
    std::string host{"tcp://127.0.0.1:3306"};
    std::string user{"root"};
    std::string password{};
    std::string database{};
    PoolConfig pool{};
  };

  /**
   * @brief Configuration parameters for a SQLite database.
   */
  struct SQLiteConfig
  {
    std::string path{"vix.db"};
    PoolConfig pool{};
  };

  /**
   * @brief Unified database configuration.
   */
  struct DbConfig
  {
    Engine engine{Engine::SQLite}; // 👈 IMPORTANT: SQLite par défaut

    MySQLConfig mysql{};
    SQLiteConfig sqlite{};
  };

  /**
   * @brief Build a DbConfig from a Vix configuration object.
   */
  DbConfig make_db_config_from_vix_config(const vix::config::Config &cfg);

  /**
   * @brief High-level database facade.
   *
   * This is the main entry point for database usage in Vix.
   *
   * Responsibilities:
   * - Select the correct driver (MySQL / SQLite)
   * - Build the connection factory
   * - Initialize and manage the connection pool
   *
   * The goal is to hide all driver-level complexity from the user.
   */
  class Database
  {
  public:
    /**
     * @brief Construct a database instance from configuration.
     *
     * @param cfg Database configuration.
     */
    explicit Database(const DbConfig &cfg);

    /**
     * @brief Create a MySQL database instance (ultra simple API).
     */
    static Database mysql(std::string host,
                          std::string user,
                          std::string password,
                          std::string database,
                          PoolConfig pool = {});

    /**
     * @brief Create a SQLite database instance (ultra simple API).
     */
    static Database sqlite(std::string path,
                           PoolConfig pool = {});

    /**
     * @brief Return the selected database engine.
     */
    Engine engine() const noexcept { return cfg_.engine; }

    /**
     * @brief Access database configuration.
     */
    const DbConfig &config() const noexcept { return cfg_; }

    /**
     * @brief Access the connection pool.
     */
    ConnectionPool &pool() noexcept { return *pool_; }

    /**
     * @brief Access the connection pool (const).
     */
    const ConnectionPool &pool() const noexcept { return *pool_; }

  private:
    DbConfig cfg_;

    /**
     * IMPORTANT:
     * On utilise un shared_ptr car ConnectionPool contient mutex + cv
     * donc non copiable.
     */
    std::shared_ptr<ConnectionPool> pool_;
  };

} // namespace vix::db

#endif // VIX_DB_DATABASE_HPP
