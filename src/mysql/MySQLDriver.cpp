/**
 *
 *  @file MySQLDriver.cpp
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
#include <vix/db/core/Errors.hpp>

#if VIX_DB_HAS_MYSQL

#include <vix/db/drivers/mysql/MySQLDriver.hpp>

#include <cppconn/exception.h>
#include <cppconn/metadata.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <memory>
#include <string>
#include <utility>
#include <variant>

namespace vix::db
{
  /**
   * @brief MySQL-backed implementation of a single result row.
   *
   * This class adapts a native sql::ResultSet row to the generic
   * vix::db::ResultRow interface.
   */
  class MySQLResultRow final : public ResultRow
  {
    sql::ResultSet *rs_ = nullptr;

  public:
    /**
     * @brief Construct an empty row wrapper.
     */
    MySQLResultRow() = default;

    /**
     * @brief Construct a row wrapper bound to a result set.
     *
     * @param rs Native MySQL result set pointer.
     */
    explicit MySQLResultRow(sql::ResultSet *rs)
        : rs_(rs) {}

    /**
     * @brief Rebind this row wrapper to another result set.
     *
     * @param rs Native MySQL result set pointer.
     */
    void reset(sql::ResultSet *rs)
    {
      rs_ = rs;
    }

    /**
     * @brief Check whether a column is NULL.
     *
     * @param i Zero-based column index.
     * @return true if the column is NULL, false otherwise.
     */
    bool isNull(std::size_t i) const override
    {
      return rs_->isNull(static_cast<unsigned int>(i + 1));
    }

    /**
     * @brief Read a column as string.
     *
     * @param i Zero-based column index.
     * @return Column value as string.
     */
    std::string getString(std::size_t i) const override
    {
      return rs_->getString(static_cast<unsigned int>(i + 1));
    }

    /**
     * @brief Read a column as 64-bit integer.
     *
     * @param i Zero-based column index.
     * @return Column value as std::int64_t.
     */
    std::int64_t getInt64(std::size_t i) const override
    {
      return static_cast<std::int64_t>(
          rs_->getInt64(static_cast<unsigned int>(i + 1)));
    }

    /**
     * @brief Read a column as double.
     *
     * @param i Zero-based column index.
     * @return Column value as double.
     */
    double getDouble(std::size_t i) const override
    {
      return static_cast<double>(
          rs_->getDouble(static_cast<unsigned int>(i + 1)));
    }
  };

  /**
   * @brief MySQL-backed implementation of a result set.
   *
   * Owns the native MySQL result set and exposes it through the generic
   * vix::db::ResultSet interface.
   */
  class MySQLResultSet final : public ResultSet
  {
    std::unique_ptr<sql::ResultSet> rs_;
    mutable MySQLResultRow row_{};

  public:
    /**
     * @brief Construct a result set wrapper.
     *
     * @param rs Native MySQL result set ownership.
     */
    explicit MySQLResultSet(std::unique_ptr<sql::ResultSet> rs)
        : rs_(std::move(rs)), row_(rs_.get()) {}

    /**
     * @brief Move to the next row.
     *
     * @return true if a row is available, false otherwise.
     */
    bool next() override
    {
      const bool ok = rs_->next();
      row_.reset(rs_.get());
      return ok;
    }

    /**
     * @brief Return the number of columns in the current result.
     *
     * @return Column count.
     */
    std::size_t cols() const override
    {
      return static_cast<std::size_t>(rs_->getMetaData()->getColumnCount());
    }

    /**
     * @brief Return the current row view.
     *
     * @return Reference to the current row adapter.
     */
    const ResultRow &row() const override
    {
      return row_;
    }
  };

  /**
   * @brief MySQL-backed prepared statement.
   *
   * Adapts sql::PreparedStatement to the generic vix::db::Statement
   * interface.
   */
  class MySQLStatement final : public Statement
  {
    std::unique_ptr<sql::PreparedStatement> ps_;

    /**
     * @brief Convert a Vix parameter index to a MySQL parameter index.
     *
     * MySQL prepared statement parameters are 1-based.
     *
     * @param i One-based parameter index expected by Vix callers.
     * @return MySQL-compatible parameter index.
     */
    static unsigned int ui(std::size_t i)
    {
      return static_cast<unsigned int>(i);
    }

    /**
     * @brief Bind a NULL value to a statement parameter.
     *
     * @param ps Prepared statement.
     * @param i  One-based parameter index.
     */
    static void bindNull(sql::PreparedStatement &ps, unsigned int i)
    {
      ps.setNull(i, 0);
    }

  public:
    /**
     * @brief Construct a MySQL prepared statement wrapper.
     *
     * @param ps Native prepared statement ownership.
     */
    explicit MySQLStatement(std::unique_ptr<sql::PreparedStatement> ps)
        : ps_(std::move(ps)) {}

    /**
     * @brief Bind a database value to a prepared statement parameter.
     *
     * @param idx One-based parameter index.
     * @param v   Database value to bind.
     */
    void bind(std::size_t idx, const DbValue &v) override
    {
      const unsigned int i = ui(idx);

      try
      {
        std::visit(
            [&](const auto &x)
            {
              using T = std::decay_t<decltype(x)>;

              if constexpr (std::is_same_v<T, std::nullptr_t>)
              {
                bindNull(*ps_, i);
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
        throw DBError(std::string("MySQL bind failed: ") + e.what());
      }
    }

    /**
     * @brief Execute the prepared statement as a query.
     *
     * @return Owning pointer to a generic result set.
     */
    std::unique_ptr<ResultSet> query() override
    {
      try
      {
        auto rs = std::unique_ptr<sql::ResultSet>(ps_->executeQuery());
        return std::make_unique<MySQLResultSet>(std::move(rs));
      }
      catch (const sql::SQLException &e)
      {
        throw DBError(std::string("MySQL query failed: ") + e.what());
      }
    }

    /**
     * @brief Execute the prepared statement as a non-query statement.
     *
     * @return Number of affected rows.
     */
    std::uint64_t exec() override
    {
      try
      {
        return static_cast<std::uint64_t>(ps_->executeUpdate());
      }
      catch (const sql::SQLException &e)
      {
        throw DBError(std::string("MySQL exec failed: ") + e.what());
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
      throw DBError(std::string("MySQL prepare failed: ") + e.what());
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
      {
        return rs->getUInt64("id");
      }

      throw DBError("No LAST_INSERT_ID()");
    }
    catch (const sql::SQLException &e)
    {
      throw DBError(std::string("MySQL lastInsertId failed: ") + e.what());
    }
  }

  std::shared_ptr<sql::Connection>
  make_mysql_conn(const std::string &host,
                  const std::string &user,
                  const std::string &pass,
                  const std::string &db)
  {
    try
    {
      auto *driver = sql::mysql::get_mysql_driver_instance();
      auto conn = std::shared_ptr<sql::Connection>(
          driver->connect(host, user, pass));

      if (!db.empty())
      {
        conn->setSchema(db);
      }

      return conn;
    }
    catch (const sql::SQLException &e)
    {
      throw DBError(std::string("MySQL connect failed: ") + e.what());
    }
  }

  ConnectionFactory make_mysql_factory(std::string host,
                                       std::string user,
                                       std::string pass,
                                       std::string db)
  {
    return [host = std::move(host),
            user = std::move(user),
            pass = std::move(pass),
            db = std::move(db)]() -> ConnectionPtr
    {
      auto raw = make_mysql_conn(host, user, pass, db);
      auto mysqlConn = std::make_shared<MySQLConnection>(std::move(raw));
      return std::static_pointer_cast<Connection>(mysqlConn);
    };
  }

} // namespace vix::db

#endif // VIX_DB_HAS_MYSQL
