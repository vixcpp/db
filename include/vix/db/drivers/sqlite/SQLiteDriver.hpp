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
  class SQLiteConnection final : public Connection
  {
    sqlite3 *db_ = nullptr;

  public:
    explicit SQLiteConnection(sqlite3 *db) : db_(db) {}
    ~SQLiteConnection() override;

    SQLiteConnection(const SQLiteConnection &) = delete;
    SQLiteConnection &operator=(const SQLiteConnection &) = delete;

    std::unique_ptr<Statement> prepare(std::string_view sql) override;

    void begin() override;
    void commit() override;
    void rollback() override;

    std::uint64_t lastInsertId() override;
    bool ping() override { return db_ != nullptr; }

    sqlite3 *raw() const { return db_; }
  };

  // Open a sqlite connection (creates DB file if needed)
  sqlite3 *open_sqlite(const std::string &path);

  // Factory for ConnectionPool / Database
  ConnectionFactory make_sqlite_factory(std::string path);

} // namespace vix::db

#endif // VIX_DB_HAS_SQLITE
#endif // VIX_DB_SQLITE_DRIVER_HPP
