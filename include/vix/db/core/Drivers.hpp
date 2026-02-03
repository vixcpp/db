/**
 *
 *  @file Drivers.hpp
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
#ifndef VIX_DB_DRIVERS_HPP
#define VIX_DB_DRIVERS_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

#include <vix/db/core/Result.hpp>
#include <vix/db/core/Value.hpp>

namespace vix::db
{
  /**
   * @brief Abstract prepared statement interface.
   *
   * A Statement represents a prepared SQL statement with positional parameters.
   * Concrete database drivers (MySQL, SQLite, PostgreSQL, etc.) must implement
   * this interface.
   *
   * The API is intentionally minimal and type-erased through DbValue to keep
   * drivers simple and consistent.
   */
  struct Statement
  {
    virtual ~Statement() = default;

    /**
     * @brief Bind a value to a positional parameter.
     *
     * Indexing is driver-defined but is expected to be 1-based for SQL drivers
     * (e.g. ?, ?, ?) to match common database conventions.
     *
     * @param idx Parameter index.
     * @param v Database value wrapper.
     */
    virtual void bind(std::size_t idx, const DbValue &v) = 0;

    /// Convenience overloads for common C++ types
    void bind(std::size_t idx, int v) { bind(idx, static_cast<std::int64_t>(v)); }
    void bind(std::size_t idx, unsigned v) { bind(idx, static_cast<std::int64_t>(v)); }
    void bind(std::size_t idx, std::int64_t v) { bind(idx, i64(v)); }
    void bind(std::size_t idx, std::uint64_t v) { bind(idx, static_cast<std::int64_t>(v)); }
    void bind(std::size_t idx, double v) { bind(idx, f64(v)); }
    void bind(std::size_t idx, bool v) { bind(idx, b(v)); }
    void bind(std::size_t idx, std::string v) { bind(idx, str(std::move(v))); }
    void bind(std::size_t idx, const char *v)
    {
      bind(idx, str(std::string(v ? v : "")));
    }

    /**
     * @brief Bind a SQL NULL value.
     *
     * @param idx Parameter index.
     */
    void bindNull(std::size_t idx) { bind(idx, null()); }

    /**
     * @brief Execute a query and return a result set.
     *
     * This is typically used for SELECT statements.
     *
     * @return Owning pointer to a ResultSet.
     */
    virtual std::unique_ptr<ResultSet> query() = 0;

    /**
     * @brief Execute a statement without returning rows.
     *
     * This is typically used for INSERT, UPDATE, DELETE.
     *
     * @return Number of affected rows, if supported by the driver.
     */
    virtual std::uint64_t exec() = 0;
  };

  /**
   * @brief Abstract database connection interface.
   *
   * A Connection represents a live connection to a database backend.
   * It is responsible for preparing statements and managing transactions.
   */
  struct Connection
  {
    virtual ~Connection() = default;

    /**
     * @brief Prepare a SQL statement.
     *
     * @param sql SQL string (UTF-8).
     * @return Owning pointer to a prepared Statement.
     */
    virtual std::unique_ptr<Statement> prepare(std::string_view sql) = 0;

    /**
     * @brief Begin a transaction.
     */
    virtual void begin() = 0;

    /**
     * @brief Commit the current transaction.
     */
    virtual void commit() = 0;

    /**
     * @brief Roll back the current transaction.
     */
    virtual void rollback() = 0;

    /**
     * @brief Return the last auto-generated insert identifier.
     *
     * Semantics depend on the underlying database (AUTO_INCREMENT, ROWID, etc.).
     */
    virtual std::uint64_t lastInsertId() = 0;

    /**
     * @brief Check whether the connection is still alive.
     *
     * Drivers may override this to implement a real ping.
     *
     * @return true if the connection is usable.
     */
    virtual bool ping() { return true; }
  };

  /// Shared pointer alias for database connections
  using ConnectionPtr = std::shared_ptr<Connection>;

  /**
   * @brief Factory function type for creating database connections.
   *
   * Typically used by connection pools or dependency injection systems.
   */
  using ConnectionFactory = std::function<ConnectionPtr()>;

} // namespace vix::db

#endif // VIX_DB_DRIVERS_HPP
