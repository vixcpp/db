/**
 *
 *  @file MySQLDriver.hpp
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
#ifndef VIX_DB_MYSQL_DRIVER_HPP
#define VIX_DB_MYSQL_DRIVER_HPP

#if VIX_DB_HAS_MYSQL

#include <vix/db/core/Drivers.hpp>

#include <cppconn/connection.h>
#include <mysql_driver.h>

#include <memory>
#include <string>

namespace vix::db
{
  /**
   * @brief MySQL implementation of a database connection.
   *
   * MySQLConnection is a concrete Connection backed by the MySQL
   * Connector/C++ driver. It adapts the native MySQL API to the
   * generic vix::db::Connection interface.
   *
   * This class is not copyable and is intended to be managed via
   * smart pointers.
   */
  class MySQLConnection final : public Connection
  {
    std::shared_ptr<sql::Connection> conn_;

  public:
    /**
     * @brief Construct a MySQL connection wrapper.
     *
     * @param c Shared pointer to a native MySQL Connector/C++ connection.
     */
    explicit MySQLConnection(std::shared_ptr<sql::Connection> c)
        : conn_(std::move(c)) {}

    /**
     * @brief Prepare a SQL statement.
     *
     * @param sql SQL query string (UTF-8).
     * @return Owning pointer to a prepared Statement.
     */
    std::unique_ptr<Statement> prepare(std::string_view sql) override;

    /**
     * @brief Begin a transaction.
     *
     * Internally disables autocommit on the MySQL connection.
     */
    void begin() override { conn_->setAutoCommit(false); }

    /**
     * @brief Commit the current transaction.
     *
     * Autocommit is re-enabled after a successful commit.
     */
    void commit() override
    {
      conn_->commit();
      conn_->setAutoCommit(true);
    }

    /**
     * @brief Roll back the current transaction.
     *
     * Autocommit is re-enabled after a rollback.
     */
    void rollback() override
    {
      conn_->rollback();
      conn_->setAutoCommit(true);
    }

    /**
     * @brief Return the last auto-incremented identifier.
     *
     * @return Last generated insert ID.
     */
    std::uint64_t lastInsertId() override;

    /**
     * @brief Check whether the underlying MySQL connection is valid.
     *
     * @return true if the connection is usable, false otherwise.
     */
    bool ping() override
    {
      try
      {
        return conn_ && conn_->isValid();
      }
      catch (...)
      {
        return false;
      }
    }

    /**
     * @brief Access the underlying native MySQL connection.
     *
     * Intended for advanced or driver-specific use cases.
     *
     * @return Shared pointer to the native sql::Connection.
     */
    const std::shared_ptr<sql::Connection> &raw() const { return conn_; }
  };

  /**
   * @brief Create a native MySQL connection.
   *
   * Establishes a connection to a MySQL server using the
   * Connector/C++ driver.
   *
   * @param host Database host.
   * @param user Username.
   * @param pass Password.
   * @param db   Database name.
   * @return Shared pointer to a native MySQL connection.
   */
  std::shared_ptr<sql::Connection>
  make_mysql_conn(const std::string &host,
                  const std::string &user,
                  const std::string &pass,
                  const std::string &db);

  /**
   * @brief Create a connection factory for MySQL connections.
   *
   * Returns a callable that produces new MySQL-backed
   * vix::db::Connection instances. This is typically used
   * by connection pools or dependency injection systems.
   *
   * @param host Database host.
   * @param user Username.
   * @param pass Password.
   * @param db   Database name.
   * @return Factory function producing Connection instances.
   */
  std::function<std::shared_ptr<Connection>()>
  make_mysql_factory(std::string host,
                     std::string user,
                     std::string pass,
                     std::string db);

} // namespace vix::db

#endif // VIX_DB_HAS_MYSQL
#endif // VIX_DB_MYSQL_DRIVER_HPP
