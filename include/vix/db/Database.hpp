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

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <vix/db/Transaction.hpp>
#include <vix/db/result/OwnedResultSet.hpp>

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
    /// Selected database engine
    Engine engine{Engine::SQLite};

    /// MySQL-specific configuration
    MySQLConfig mysql{};

    /// SQLite-specific configuration
    SQLiteConfig sqlite{};
  };

  /**
   * @brief High-level database facade.
   *
   * Database is the main entry point for database usage in Vix.
   *
   * Responsibilities:
   * - Select the correct driver (MySQL / SQLite)
   * - Build the connection factory
   * - Initialize and manage the connection pool
   * - Expose a simpler public API for common queries
   *
   * The goal is to keep driver and pooling complexity inside Vix
   * while providing a minimal API to applications.
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
    explicit Database(const vix::config::Config &cfg);

    /**
     * @brief Create a MySQL database instance.
     *
     * @param host MySQL host string.
     * @param user Database username.
     * @param password Database password.
     * @param database Database name.
     * @param pool Pool configuration.
     * @return Configured database instance.
     */
    static Database mysql(std::string host,
                          std::string user,
                          std::string password,
                          std::string database,
                          PoolConfig pool = {});

    /**
     * @brief Create a SQLite database instance.
     *
     * @param path SQLite database file path.
     * @param pool Pool configuration.
     * @return Configured database instance.
     */
    static Database sqlite(std::string path,
                           PoolConfig pool = {});

    /**
     * @brief Execute a SQL statement without returning rows.
     *
     * This helper internally acquires a pooled connection,
     * prepares the statement, binds all positional arguments,
     * then executes it.
     *
     * Typical use cases:
     * - CREATE TABLE
     * - INSERT
     * - UPDATE
     * - DELETE
     *
     * @tparam Args Bound argument types.
     * @param sql SQL statement.
     * @param args Positional bind values.
     * @return Number of affected rows, when supported by the driver.
     */
    template <typename... Args>
    std::uint64_t exec(std::string_view sql, Args &&...args)
    {
      PooledConn conn(pool());

      auto stmt = conn->prepare(sql);

      std::size_t i = 1;
      (stmt->bind(i++, std::forward<Args>(args)), ...);

      return stmt->exec();
    }

    /**
     * @brief Execute a SQL query and return a safe result set.
     *
     * This helper acquires a pooled connection, prepares the statement,
     * binds all positional arguments, executes the query, then returns
     * a result wrapper that keeps the connection alive until the result
     * set is destroyed by the caller.
     *
     * @tparam Args Bound argument types.
     * @param sql SQL query.
     * @param args Positional bind values.
     * @return Owning pointer to a safe result set.
     */
    template <typename... Args>
    std::unique_ptr<ResultSet> query(std::string_view sql, Args &&...args)
    {
      PooledConn conn(pool());

      auto stmt = conn->prepare(sql);

      std::size_t i = 1;
      (stmt->bind(i++, std::forward<Args>(args)), ...);

      auto result = stmt->query();

      return std::make_unique<OwnedResultSet>(
          std::move(conn),
          std::move(result));
    }

    /**
     * @brief Execute a callback inside a transaction.
     *
     * A Transaction is started before invoking the callback.
     * If the callback completes successfully, the transaction
     * is committed. If it throws, the transaction is rolled back
     * and the exception is rethrown.
     *
     * The callback receives a reference to the underlying Connection.
     *
     * @tparam Fn Callback type.
     * @param fn Callback executed inside the transaction.
     * @return The value returned by the callback, if any.
     */
    template <typename Fn>
    auto transaction(Fn &&fn) -> decltype(fn(std::declval<Connection &>()))
    {
      Transaction tx(pool());

      try
      {
        using ReturnType = decltype(fn(std::declval<Connection &>()));

        if constexpr (std::is_void_v<ReturnType>)
        {
          fn(tx.conn());
          tx.commit();
          return;
        }
        else
        {
          auto result = fn(tx.conn());
          tx.commit();
          return result;
        }
      }
      catch (...)
      {
        tx.rollback();
        throw;
      }
    }

    /**
     * @brief Return the selected database engine.
     *
     * @return Active engine.
     */
    Engine engine() const noexcept { return cfg_.engine; }

    /**
     * @brief Access database configuration.
     *
     * @return Database configuration.
     */
    const DbConfig &config() const noexcept { return cfg_; }

    /**
     * @brief Access the connection pool.
     *
     * This is mainly intended for advanced usage.
     *
     * @return Mutable reference to the connection pool.
     */
    ConnectionPool &pool() noexcept { return *pool_; }

    /**
     * @brief Access the connection pool.
     *
     * This is mainly intended for advanced usage.
     *
     * @return Const reference to the connection pool.
     */
    const ConnectionPool &pool() const noexcept { return *pool_; }

  private:
    /// Database configuration stored by the facade
    DbConfig cfg_;

    /**
     * @brief Shared connection pool instance.
     *
     * A shared_ptr is used because ConnectionPool contains
     * synchronization primitives and is not copyable.
     */
    std::shared_ptr<ConnectionPool> pool_;
  };

} // namespace vix::db

#endif // VIX_DB_DATABASE_HPP
