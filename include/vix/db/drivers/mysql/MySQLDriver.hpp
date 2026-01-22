/**
 *
 *  @file MySQLDriver.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
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
  class MySQLConnection final : public Connection
  {
    std::shared_ptr<sql::Connection> conn_;

  public:
    explicit MySQLConnection(std::shared_ptr<sql::Connection> c)
        : conn_(std::move(c)) {}

    std::unique_ptr<Statement> prepare(std::string_view sql) override;

    void begin() override { conn_->setAutoCommit(false); }

    void commit() override
    {
      conn_->commit();
      conn_->setAutoCommit(true);
    }

    void rollback() override
    {
      conn_->rollback();
      conn_->setAutoCommit(true);
    }

    std::uint64_t lastInsertId() override;

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

    const std::shared_ptr<sql::Connection> &raw() const { return conn_; }
  };

  std::shared_ptr<sql::Connection>
  make_mysql_conn(const std::string &host,
                  const std::string &user,
                  const std::string &pass,
                  const std::string &db);

  std::function<std::shared_ptr<Connection>()>
  make_mysql_factory(std::string host,
                     std::string user,
                     std::string pass,
                     std::string db);

} // namespace vix::db

#endif // VIX_DB_HAS_MYSQL
#endif // VIX_DB_MYSQL_DRIVER_HPP
