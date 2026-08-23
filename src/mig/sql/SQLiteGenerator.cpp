#include <vix/db/mig/sql/SQLiteGenerator.hpp>

#include <sstream>
#include <stdexcept>

namespace vix::db::mig::sql
{
  using namespace vix::db::schema;
  using namespace vix::db::mig::diff;

  static std::string quote_identifier(const std::string &ident)
  {
    std::string out;
    out.reserve(ident.size() + 2);
    out.push_back('"');
    for (const char ch : ident)
    {
      if (ch == '"')
        out += "\"\"";
      else
        out.push_back(ch);
    }
    out.push_back('"');
    return out;
  }

  static std::string type_sqlite(const Type &t)
  {
    switch (t.base)
    {
    case BaseType::Int:
    case BaseType::BigInt:
    case BaseType::Bool:
      return "INTEGER";
    case BaseType::Double:
      return "REAL";
    case BaseType::VarChar:
    case BaseType::Text:
    case BaseType::DateTime:
      return "TEXT";
    }
    return "TEXT";
  }

  static bool is_integer_type(const Type &t)
  {
    return t.base == BaseType::Int || t.base == BaseType::BigInt;
  }

  static std::vector<std::string> primary_key_columns(const Table &t)
  {
    std::vector<std::string> keys;
    for (const auto &c : t.columns)
      if (c.primary_key)
        keys.push_back(c.name);
    return keys;
  }

  static bool is_inline_integer_primary_key(const Column &c, const Table &t)
  {
    if (!c.primary_key || !is_integer_type(c.type))
      return false;
    return primary_key_columns(t).size() == 1;
  }

  static void validate_auto_increment(const Column &c, const Table &t)
  {
    if (!c.auto_increment)
      return;
    if (!is_inline_integer_primary_key(c, t))
    {
      throw std::runtime_error(
          "SQLite AUTOINCREMENT requires a single INTEGER PRIMARY KEY column: " + c.name);
    }
  }

  static std::string column_sqlite(const Column &c, const Table *table_context)
  {
    if (table_context != nullptr)
      validate_auto_increment(c, *table_context);
    else if (c.auto_increment)
      throw std::runtime_error("SQLite ADD COLUMN does not support AUTOINCREMENT: " + c.name);

    std::ostringstream o;
    o << quote_identifier(c.name) << " " << type_sqlite(c.type);

    if (table_context != nullptr && is_inline_integer_primary_key(c, *table_context))
    {
      o << " PRIMARY KEY";
      if (c.auto_increment)
        o << " AUTOINCREMENT";
    }
    else if (!c.nullable)
    {
      o << " NOT NULL";
    }

    if (c.def)
      o << " DEFAULT " << c.def->sql_literal;

    if (c.unique && !c.primary_key)
      o << " UNIQUE";

    return o.str();
  }

  static void validate_add_column_sqlite(const Column &c)
  {
    if (c.primary_key)
      throw std::runtime_error("SQLite ADD COLUMN cannot add a PRIMARY KEY column: " + c.name);
    if (c.unique)
      throw std::runtime_error("SQLite ADD COLUMN cannot add a UNIQUE column: " + c.name);
    if (c.auto_increment)
      throw std::runtime_error("SQLite ADD COLUMN cannot add an AUTOINCREMENT column: " + c.name);
    if (!c.nullable && !c.def)
    {
      throw std::runtime_error(
          "SQLite ADD COLUMN cannot add a NOT NULL column without a DEFAULT: " + c.name);
    }
  }

  static std::string create_table_sqlite(const Table &t)
  {
    std::ostringstream o;
    o << "CREATE TABLE IF NOT EXISTS " << quote_identifier(t.name) << " (\n";

    const auto keys = primary_key_columns(t);
    const bool table_level_pk = keys.size() > 1 ||
                                (keys.size() == 1 &&
                                 !is_inline_integer_primary_key(*t.findColumn(keys.front()), t));

    for (size_t i = 0; i < t.columns.size(); ++i)
    {
      o << "  " << column_sqlite(t.columns[i], &t);
      if (i + 1 < t.columns.size() || table_level_pk)
        o << ",";
      o << "\n";
    }

    if (table_level_pk)
    {
      o << "  PRIMARY KEY (";
      for (size_t i = 0; i < keys.size(); ++i)
      {
        o << quote_identifier(keys[i]);
        if (i + 1 < keys.size())
          o << ", ";
      }
      o << ")\n";
    }

    o << ");";
    return o.str();
  }

  static std::string drop_table_sqlite(const Table &t)
  {
    return "DROP TABLE IF EXISTS " + quote_identifier(t.name) + ";";
  }

  static std::string add_column_sqlite(const std::string &table, const Column &c)
  {
    validate_add_column_sqlite(c);
    return "ALTER TABLE " + quote_identifier(table) + " ADD COLUMN " + column_sqlite(c, nullptr) + ";";
  }

  static std::string drop_column_sqlite(const std::string &table, const Column &c)
  {
    return "ALTER TABLE " + quote_identifier(table) + " DROP COLUMN " + quote_identifier(c.name) + ";";
  }

  static std::string create_index_sqlite(const std::string &table, const Index &i)
  {
    if (i.columns.empty())
      throw std::runtime_error("SQLite CREATE INDEX requires at least one column: " + i.name);

    std::ostringstream o;
    o << "CREATE ";
    if (i.unique)
      o << "UNIQUE ";
    o << "INDEX IF NOT EXISTS " << quote_identifier(i.name)
      << " ON " << quote_identifier(table) << " (";
    for (size_t k = 0; k < i.columns.size(); ++k)
    {
      o << quote_identifier(i.columns[k]);
      if (k + 1 < i.columns.size())
        o << ", ";
    }
    o << ");";
    return o.str();
  }

  static std::string drop_index_sqlite(const Index &i)
  {
    return "DROP INDEX IF EXISTS " + quote_identifier(i.name) + ";";
  }

  static std::string render_up(const Op &op)
  {
    return std::visit([](auto &&x) -> std::string
                      {
      using T = std::decay_t<decltype(x)>;
      if constexpr (std::is_same_v<T, CreateTable>) return create_table_sqlite(x.table);
      else if constexpr (std::is_same_v<T, DropTable>) return drop_table_sqlite(x.table);
      else if constexpr (std::is_same_v<T, AddColumn>) return add_column_sqlite(x.table, x.column);
      else if constexpr (std::is_same_v<T, DropColumn>) return drop_column_sqlite(x.table, x.column);
      else if constexpr (std::is_same_v<T, CreateIndex>) return create_index_sqlite(x.table, x.index);
      else if constexpr (std::is_same_v<T, DropIndex>) return drop_index_sqlite(x.index); }, op);
  }

  static std::string render_down(const Op &op)
  {
    return std::visit([](auto &&x) -> std::string
                      {
      using T = std::decay_t<decltype(x)>;
      if constexpr (std::is_same_v<T, CreateTable>) return drop_table_sqlite(x.table);
      else if constexpr (std::is_same_v<T, DropTable>) return create_table_sqlite(x.table);
      else if constexpr (std::is_same_v<T, AddColumn>) return drop_column_sqlite(x.table, x.column);
      else if constexpr (std::is_same_v<T, DropColumn>) return add_column_sqlite(x.table, x.column);
      else if constexpr (std::is_same_v<T, CreateIndex>) return drop_index_sqlite(x.index);
      else if constexpr (std::is_same_v<T, DropIndex>) return create_index_sqlite(x.table, x.index); }, op);
  }

  static std::string render_script(const char *header,
                                   const std::vector<Op> &ops,
                                   bool down)
  {
    std::vector<std::string> statements;
    statements.reserve(ops.size());

    if (down)
    {
      for (auto it = ops.rbegin(); it != ops.rend(); ++it)
        statements.push_back(render_down(*it));
    }
    else
    {
      for (const auto &op : ops)
        statements.push_back(render_up(op));
    }

    std::ostringstream o;
    o << header << "\n";
    for (const auto &statement : statements)
      o << statement << "\n";
    return o.str();
  }

  std::string to_sqlite_up(const std::vector<Op> &ops)
  {
    return render_script("-- Generated by Vix ORM (SQLite)", ops, false);
  }

  std::string to_sqlite_down(const std::vector<Op> &ops)
  {
    return render_script("-- Generated by Vix ORM (SQLite) [DOWN]", ops, true);
  }

} // namespace vix::db::mig::sql
