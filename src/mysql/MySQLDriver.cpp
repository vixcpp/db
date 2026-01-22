/**
 *
 *  @file MySQLDriver.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#include <vix/db/core/Errors.hpp>

#if VIX_DB_HAS_MYSQL

#include <vix/db/drivers/mysql/MySQLDriver.hpp>

#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#include <memory>
#include <string>
#include <utility>
#include <variant>

namespace vix::db
{
  class MySQLResultRow final : public ResultRow
  {
    sql::ResultSet *rs_ = nullptr;

  public:
    MySQLResultRow() = default;
    explicit MySQLResultRow(sql::ResultSet *rs) : rs_(rs) {}

    void reset(sql::ResultSet *rs) { rs_ = rs; }

    bool isNull(std::size_t i) const override
    {
      return rs_->isNull(static_cast<unsigned int>(i + 1));
    }

    std::string getString(std::size_t i) const override
    {
      return rs_->getString(static_cast<unsigned int>(i + 1));
    }

    std::int64_t getInt64(std::size_t i) const override
    {
      return static_cast<std::int64_t>(
          rs_->getInt64(static_cast<unsigned int>(i + 1)));
    }

    double getDouble(std::size_t i) const override
    {
      return static_cast<double>(
          rs_->getDouble(static_cast<unsigned int>(i + 1)));
    }
  };

  class MySQLResultSet final : public ResultSet
  {
    std::unique_ptr<sql::ResultSet> rs_;
    mutable MySQLResultRow row_{};

  public:
    explicit MySQLResultSet(std::unique_ptr<sql::ResultSet> rs)
        : rs_(std::move(rs)), row_(rs_.get()) {}

    bool next() override
    {
      const bool ok = rs_->next();
      row_.reset(rs_.get());
      return ok;
    }

    std::size_t cols() const override
    {
      return static_cast<std::size_t>(rs_->getMetaData()->getColumnCount());
    }

    const ResultRow &row() const override
    {
      return row_;
    }
  };

  class MySQLStatement final : public Statement
  {
    std::unique_ptr<sql::PreparedStatement> ps_;

    static unsigned int ui(std::size_t i)
    {
      // SQL params are 1-based: 1,2,3...
      return static_cast<unsigned int>(i);
    }

    static void bind_null(sql::PreparedStatement &ps, unsigned int i)
    {
      ps.setNull(i, 0);
    }

  public:
    explicit MySQLStatement(std::unique_ptr<sql::PreparedStatement> ps)
        : ps_(std::move(ps)) {}

    void bind(std::size_t idx, const DbValue &v) override
    {
      const auto i = ui(idx);

      try
      {
        std::visit(
            [&](const auto &x)
            {
              using T = std::decay_t<decltype(x)>;

              if constexpr (std::is_same_v<T, std::nullptr_t>)
              {
                bind_null(*ps_, i);
              }
              else if constexpr (std::is_same_v<T, bool>)
              {
                ps_->setBoolean(i, x);
              }
              else if constexpr (std::is_same_v<T, std::int64_t>)
              {
                ps_->setInt64(i, x);
              }
              else if constexpr (std::is_same_v<T, double>)
              {
                ps_->setDouble(i, x);
              }
              else if constexpr (std::is_same_v<T, std::string>)
              {
                ps_->setString(i, x);
              }
              else if constexpr (std::is_same_v<T, Blob>)
              {
                throw DBError("MySQL bind Blob not implemented yet");
              }
              else
              {
                throw DBError("Unsupported DbValue variant in MySQLStatement::bind");
              }
            },
            v);
      }
      catch (const sql::SQLException &e)
      {
        throw DBError(std::string{"MySQL bind failed: "} + e.what());
      }
    }

    std::unique_ptr<ResultSet> query() override
    {
      try
      {
        auto rs = std::unique_ptr<sql::ResultSet>(ps_->executeQuery());
        return std::make_unique<MySQLResultSet>(std::move(rs));
      }
      catch (const sql::SQLException &e)
      {
        throw DBError(std::string{"MySQL query failed: "} + e.what());
      }
    }

    std::uint64_t exec() override
    {
      try
      {
        return static_cast<std::uint64_t>(ps_->executeUpdate());
      }
      catch (const sql::SQLException &e)
      {
        throw DBError(std::string{"MySQL exec failed: "} + e.what());
      }
    }
  };

  std::unique_ptr<Statement> MySQLConnection::prepare(std::string_view sql)
  {
    try
    {
      auto ps = std::unique_ptr<sql::PreparedStatement>(
          conn_->prepareStatement(std::string(sql)));
      return std::make_unique<MySQLStatement>(std::move(ps));
    }
    catch (const sql::SQLException &e)
    {
      throw DBError(std::string{"MySQL prepare failed: "} + e.what());
    }
  }

  std::uint64_t MySQLConnection::lastInsertId()
  {
    try
    {
      auto st = std::unique_ptr<sql::Statement>(conn_->createStatement());
      auto rs = std::unique_ptr<sql::ResultSet>(
          st->executeQuery("SELECT LAST_INSERT_ID() AS id"));
      if (rs->next())
        return rs->getUInt64("id");
      throw DBError("No LAST_INSERT_ID()");
    }
    catch (const sql::SQLException &e)
    {
      throw DBError(std::string{"MySQL lastInsertId failed: "} + e.what());
    }
  }

  std::shared_ptr<sql::Connection> make_mysql_conn(
      const std::string &host,
      const std::string &user,
      const std::string &pass,
      const std::string &db)
  {
    try
    {
      auto *driver = sql::mysql::get_mysql_driver_instance();
      auto c = std::shared_ptr<sql::Connection>(driver->connect(host, user, pass));
      if (!db.empty())
        c->setSchema(db);
      return c;
    }
    catch (const sql::SQLException &e)
    {
      throw DBError(std::string{"MySQL connect failed: "} + e.what());
    }
  }

  std::function<std::shared_ptr<Connection>()>
  make_mysql_factory(
      std::string host,
      std::string user,
      std::string pass,
      std::string db)
  {
    return [host = std::move(host),
            user = std::move(user),
            pass = std::move(pass),
            db = std::move(db)]() -> std::shared_ptr<Connection>
    {
      auto raw = make_mysql_conn(host, user, pass, db);
      auto mysql_conn = std::make_shared<MySQLConnection>(std::move(raw));
      return std::static_pointer_cast<Connection>(mysql_conn);
    };
  }

} // namespace vix::db

#endif // VIX_DB_HAS_MYSQL
