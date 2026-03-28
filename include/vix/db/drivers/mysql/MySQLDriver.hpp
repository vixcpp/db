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

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace vix::db
{
  /**
   * @brief MySQL implementation of a database connection.
   *
   * MySQLConnection is a concrete implementation of the generic
   * vix::db::Connection interface backed by MySQL Connector/C++.
   *
   * It adapts the native MySQL driver API to the uniform Vix database
   * abstraction so higher-level layers such as ORM, repositories, and
   * connection pools can work without depending directly on MySQL APIs.
   *
   * This type is intended to be managed through smart pointers.
   */
  class MySQLConnection final : public Connection
  {
    std::shared_ptr<sql::Connection> conn_;

  public:
    /**
     * @brief Construct a MySQL connection wrapper.
     *
     * @param c Shared pointer to the native MySQL connection.
     */
    explicit MySQLConnection(std::shared_ptr<sql::Connection> c)
        : conn_(std::move(c)) {}

    /**
     * @brief Prepare a SQL statement.
     *
     * Converts a SQL string into a prepared statement handled by the
     * underlying MySQL driver.
     *
     * @param sql SQL query string.
     * @return Owning pointer to a prepared statement.
     */
    std::unique_ptr<Statement> prepare(std::string_view sql) override;

    /**
     * @brief Begin a transaction.
     *
     * Internally disables autocommit on the MySQL connection.
     */
    void begin() override
    {
      conn_->setAutoCommit(false);
    }

    /**
     * @brief Commit the current transaction.
     *
     * Commits the transaction and restores autocommit mode.
     */
    void commit() override
    {
      conn_->commit();
      conn_->setAutoCommit(true);
    }

    /**
     * @brief Roll back the current transaction.
     *
     * Rolls back the transaction and restores autocommit mode.
     */
    void rollback() override
    {
      conn_->rollback();
      conn_->setAutoCommit(true);
    }

    /**
     * @brief Return the last auto-generated insert identifier.
     *
     * Typically used after an INSERT statement on a table with an
     * auto-increment primary key.
     *
     * @return Last generated insert ID.
     */
    std::uint64_t lastInsertId() override;

    /**
     * @brief Check whether the underlying MySQL connection is alive.
     *
     * This method is used by higher-level systems such as connection
     * pools to validate a connection before reuse.
     *
     * @return true if the connection is valid, false otherwise.
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
     * This is intended for advanced or driver-specific integration
     * scenarios that require direct access to Connector/C++ APIs.
     *
     * @return Const reference to the wrapped native connection.
     */
    const std::shared_ptr<sql::Connection> &raw() const
    {
      return conn_;
    }
  };

  /**
   * @brief Create a native MySQL connection.
   *
   * Establishes a connection to a MySQL server using MySQL Connector/C++.
   * If a database name is provided, the connection schema is set
   * immediately after connection.
   *
   * This function returns the native driver connection and is mainly
   * intended for low-level use or for building higher-level Vix
   * abstractions.
   *
   * @param host Database server host string.
   * @param user Database username.
   * @param pass Database password.
   * @param db   Database name to select after connection.
   * @return Shared pointer to the native MySQL connection.
   */
  std::shared_ptr<sql::Connection>
  make_mysql_conn(const std::string &host,
                  const std::string &user,
                  const std::string &pass,
                  const std::string &db);

  /**
   * @brief Create a Vix-compatible connection factory for MySQL.
   *
   * Returns a callable factory that creates MySQL-backed
   * vix::db::Connection instances. This is the preferred entry point
   * for systems such as ConnectionPool, repositories, dependency
   * injection containers, and higher-level database bootstrapping.
   *
   * By exposing a factory instead of a raw connection, Vix keeps the
   * complexity of driver instantiation inside the framework and allows
   * examples and applications to stay minimal.
   *
   * @param host Database server host string.
   * @param user Database username.
   * @param pass Database password.
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
