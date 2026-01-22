/**
 *
 *  @file SQLiteDriver.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  https://github.com/vixcpp/vix
 *  MIT license.
 *
 *  Vix.cpp
 */
#include <vix/db/core/Errors.hpp>

#if VIX_DB_HAS_SQLITE
#include <vix/db/drivers/sqlite/SQLiteDriver.hpp>

#include <cstring>

namespace vix::db
{
  static void throw_sqlite(sqlite3 *db, const char *prefix)
  {
    const char *msg = db ? sqlite3_errmsg(db) : "sqlite error";
    throw DBError(std::string(prefix) + ": " + msg);
  }

  // -------------------- Result --------------------

  class SQLiteResultRow final : public ResultRow
  {
    sqlite3_stmt *stmt_ = nullptr;

  public:
    explicit SQLiteResultRow(sqlite3_stmt *stmt) : stmt_(stmt) {}

    bool isNull(std::size_t i) const override
    {
      return sqlite3_column_type(stmt_, static_cast<int>(i)) == SQLITE_NULL;
    }

    std::string getString(std::size_t i) const override
    {
      const unsigned char *txt = sqlite3_column_text(stmt_, static_cast<int>(i));
      if (!txt)
        return {};
      return std::string(reinterpret_cast<const char *>(txt));
    }

    std::int64_t getInt64(std::size_t i) const override
    {
      return static_cast<std::int64_t>(sqlite3_column_int64(stmt_, static_cast<int>(i)));
    }

    double getDouble(std::size_t i) const override
    {
      return sqlite3_column_double(stmt_, static_cast<int>(i));
    }
  };

  class SQLiteResultSet final : public ResultSet
  {
    sqlite3_stmt *stmt_ = nullptr;
    mutable SQLiteResultRow row_;
    bool has_row_ = false;

  public:
    explicit SQLiteResultSet(sqlite3_stmt *stmt)
        : stmt_(stmt), row_(stmt) {}

    ~SQLiteResultSet() override
    {
      if (stmt_)
        sqlite3_finalize(stmt_);
    }

    bool next() override
    {
      if (!stmt_)
        return false;

      const int rc = sqlite3_step(stmt_);
      if (rc == SQLITE_ROW)
      {
        has_row_ = true;
        return true;
      }
      if (rc == SQLITE_DONE)
      {
        has_row_ = false;
        return false;
      }
      throw_sqlite(sqlite3_db_handle(stmt_), "SQLite step failed");
      return false;
    }

    std::size_t cols() const override
    {
      return stmt_ ? static_cast<std::size_t>(sqlite3_column_count(stmt_)) : 0;
    }

    const ResultRow &row() const override
    {
      if (!has_row_)
        throw DBError("SQLiteResultSet::row() called before next()");
      return row_;
    }
  };

  // -------------------- Statement --------------------

  class SQLiteStatement final : public Statement
  {
    sqlite3 *db_ = nullptr;
    sqlite3_stmt *stmt_ = nullptr;

    static int idx1(std::size_t i)
    {
      // parameters are 1-based in sqlite bind APIs
      return static_cast<int>(i);
    }

  public:
    SQLiteStatement(sqlite3 *db, sqlite3_stmt *stmt)
        : db_(db), stmt_(stmt) {}

    ~SQLiteStatement() override
    {
      if (stmt_)
        sqlite3_finalize(stmt_);
    }

    void bind(std::size_t idx, const DbValue &v) override
    {
      if (!stmt_)
        throw DBError("SQLiteStatement::bind on null stmt");

      const int i = idx1(idx);

      const int rc = std::visit(
          [&](auto &&val) -> int
          {
            using T = std::decay_t<decltype(val)>;

            if constexpr (std::is_same_v<T, std::nullptr_t>)
            {
              return sqlite3_bind_null(stmt_, i);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
              return sqlite3_bind_int(stmt_, i, val ? 1 : 0);
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
              return sqlite3_bind_int64(stmt_, i, static_cast<sqlite3_int64>(val));
            }
            else if constexpr (std::is_same_v<T, double>)
            {
              return sqlite3_bind_double(stmt_, i, val);
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
              // SQLITE_TRANSIENT => sqlite copies the bytes
              return sqlite3_bind_text(stmt_, i, val.c_str(), static_cast<int>(val.size()), SQLITE_TRANSIENT);
            }
            else if constexpr (std::is_same_v<T, Blob>)
            {
              const void *p = val.bytes.empty() ? nullptr : val.bytes.data();
              const int n = static_cast<int>(val.bytes.size());
              return sqlite3_bind_blob(stmt_, i, p, n, SQLITE_TRANSIENT);
            }
            else
            {
              return SQLITE_MISUSE;
            }
          },
          v);

      if (rc != SQLITE_OK)
        throw_sqlite(db_, "SQLite bind failed");
    }

    std::unique_ptr<ResultSet> query() override
    {
      if (!stmt_)
        throw DBError("SQLiteStatement::query on null stmt");

      // query returns ResultSet that owns stmt_
      auto *st = stmt_;
      stmt_ = nullptr;
      return std::make_unique<SQLiteResultSet>(st);
    }

    std::uint64_t exec() override
    {
      if (!stmt_)
        throw DBError("SQLiteStatement::exec on null stmt");

      const int rc = sqlite3_step(stmt_);
      if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        throw_sqlite(db_, "SQLite exec failed");

      const auto changes = static_cast<std::uint64_t>(sqlite3_changes(db_));

      // reset for reuse
      sqlite3_reset(stmt_);
      sqlite3_clear_bindings(stmt_);
      return changes;
    }
  };

  // -------------------- Connection --------------------

  SQLiteConnection::~SQLiteConnection()
  {
    if (db_)
      sqlite3_close(db_);
  }

  std::unique_ptr<Statement> SQLiteConnection::prepare(std::string_view sql)
  {
    if (!db_)
      throw DBError("SQLiteConnection::prepare on null db");

    sqlite3_stmt *stmt = nullptr;
    const int rc = sqlite3_prepare_v2(
        db_,
        sql.data(),
        static_cast<int>(sql.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK || !stmt)
      throw_sqlite(db_, "SQLite prepare failed");

    return std::make_unique<SQLiteStatement>(db_, stmt);
  }

  void SQLiteConnection::begin()
  {
    prepare("BEGIN")->exec();
  }

  void SQLiteConnection::commit()
  {
    prepare("COMMIT")->exec();
  }

  void SQLiteConnection::rollback()
  {
    prepare("ROLLBACK")->exec();
  }

  std::uint64_t SQLiteConnection::lastInsertId()
  {
    if (!db_)
      throw DBError("SQLiteConnection::lastInsertId on null db");
    return static_cast<std::uint64_t>(sqlite3_last_insert_rowid(db_));
  }

  sqlite3 *open_sqlite(const std::string &path)
  {
    sqlite3 *db = nullptr;
    const int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK || !db)
    {
      // sqlite3_open may allocate db even on error; close it
      if (db)
        sqlite3_close(db);
      throw DBError("SQLite open failed for: " + path);
    }

    // Basic sane defaults (safe + practical)
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA journal_mode = WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA synchronous = NORMAL;", nullptr, nullptr, nullptr);

    return db;
  }

  ConnectionFactory make_sqlite_factory(std::string path)
  {
    return [path = std::move(path)]() -> ConnectionPtr
    {
      sqlite3 *db = open_sqlite(path);
      auto c = std::make_shared<SQLiteConnection>(db);
      return std::static_pointer_cast<Connection>(c);
    };
  }

} // namespace vix::db

#endif // VIX_DB_HAS_SQLITE
