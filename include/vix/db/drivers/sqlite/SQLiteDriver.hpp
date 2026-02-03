/**
 *
 *  @file SQLiteDriver.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  https://github.com/vixcpp/vix
 *  MIT license.
 *
 *  Vix.cpp
 */
#ifndef VIX_DB_SQLITE_DRIVER_HPP
#define VIX_DB_SQLITE_DRIVER_HPP

#if VIX_DB_HAS_SQLITE

#include <vix/db/core/Drivers.hpp>

#include <sqlite3.h>

#include <memory>
#include <string>

namespace vix::db
{
  /**
   * @brief SQLite implementation of a database connection.
   *
   * SQLiteConnection adapts the SQLite C API to the generic
   * vix::db::Connection interface. It manages the lifetime of
   * the underlying sqlite3 handle.
   *
   * This connection is not copyable.
   */
  class SQLiteConnection final : public Connection
  {
    sqlite3 *db_ = nullptr;

  public:
    /**
     * @brief Construct a SQLite connection wrapper.
     *
     * @param db Raw sqlite3 handle.
     */
    explicit SQLiteConnection(sqlite3 *db) : db_(db) {}

    /**
     * @brief Destroy the SQLite connection.
     *
     * Closes the underlying sqlite3 handle.
     */
    ~SQLiteConnection() override;

    SQLiteConnection(const SQLiteConnection &) = delete;
    SQLiteConnection &operator=(const SQLiteConnection &) = delete;

    /**
     * @brief Prepare a SQL statement.
     *
     * @param sql SQL query string (UTF-8).
     * @return Owning pointer to a prepared Statement.
     */
    std::unique_ptr<Statement> prepare(std::string_view sql) override;

    /**
     * @brief Begin a transaction.
     */
    void begin() override;

    /**
     * @brief Commit the current transaction.
     */
    void commit() override;

    /**
     * @brief Roll back the current transaction.
     */
    void rollback() override;

    /**
     * @brief Return the last inserted row identifier.
     *
     * @return Last inserted ROWID.
     */
    std::uint64_t lastInsertId() override;

    /**
     * @brief Check whether the connection is valid.
     *
     * @return true if the underlying sqlite3 handle exists.
     */
    bool ping() override { return db_ != nullptr; }

    /**
     * @brief Access the underlying sqlite3 handle.
     *
     * Intended for advanced or driver-specific use cases.
     *
     * @return Raw sqlite3 pointer.
     */
    sqlite3 *raw() const { return db_; }
  };

  /**
   * @brief Open a SQLite database connection.
   *
   * Creates the database file if it does not already exist.
   *
   * @param path Path to the SQLite database file.
   * @return Raw sqlite3 handle.
   */
  sqlite3 *open_sqlite(const std::string &path);

  /**
   * @brief Create a connection factory for SQLite connections.
   *
   * Returns a factory producing new SQLite-backed
   * vix::db::Connection instances. Commonly used by
   * connection pools or database abstractions.
   *
   * @param path Path to the SQLite database file.
   * @return Connection factory.
   */
  ConnectionFactory make_sqlite_factory(std::string path);

} // namespace vix::db

#endif // VIX_DB_HAS_SQLITE
#endif // VIX_DB_SQLITE_DRIVER_HPP
