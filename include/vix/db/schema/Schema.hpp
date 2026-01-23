#ifndef VIX_DB_SCHEMA_SCHEMA_HPP
#define VIX_DB_SCHEMA_SCHEMA_HPP

#include <vix/db/schema/Types.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace vix::db::schema
{
  struct Column
  {
    std::string name;
    Type type;

    bool nullable = true;
    bool primary_key = false;
    bool auto_increment = false;
    bool unique = false;

    std::optional<DefaultValue> def;
  };

  struct Index
  {
    std::string name;
    std::vector<std::string> columns;
    bool unique = false;
  };

  struct Table
  {
    std::string name;
    std::vector<Column> columns;
    std::vector<Index> indexes;

    const Column *findColumn(const std::string &n) const
    {
      for (const auto &c : columns)
        if (c.name == n)
          return &c;
      return nullptr;
    }

    Column *findColumn(const std::string &n)
    {
      for (auto &c : columns)
        if (c.name == n)
          return &c;
      return nullptr;
    }

    const Index *findIndex(const std::string &n) const
    {
      for (const auto &i : indexes)
        if (i.name == n)
          return &i;
      return nullptr;
    }
  };

  struct Schema
  {
    std::vector<Table> tables;

    const Table *findTable(const std::string &n) const
    {
      for (const auto &t : tables)
        if (t.name == n)
          return &t;
      return nullptr;
    }

    Table *findTable(const std::string &n)
    {
      for (auto &t : tables)
        if (t.name == n)
          return &t;
      return nullptr;
    }
  };

} // namespace vix::db::schema

#endif
