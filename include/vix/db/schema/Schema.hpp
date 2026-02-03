/**
 *
 *  @file Schema.hpp
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
  /**
   * @brief Column definition within a database table.
   *
   * Describes the name, type, and constraints of a table column.
   * Column definitions are used for schema diffing, migration
   * generation, and SQL emission.
   */
  struct Column
  {
    /// Column name
    std::string name;

    /// Column data type
    Type type;

    /// Whether the column allows NULL values
    bool nullable = true;

    /// Whether the column is part of the primary key
    bool primary_key = false;

    /// Whether the column auto-increments (engine-specific)
    bool auto_increment = false;

    /// Whether the column has a uniqueness constraint
    bool unique = false;

    /// Optional default value
    std::optional<DefaultValue> def;
  };

  /**
   * @brief Index definition.
   *
   * Represents a database index over one or more columns.
   */
  struct Index
  {
    /// Index name
    std::string name;

    /// Ordered list of indexed column names
    std::vector<std::string> columns;

    /// Whether the index enforces uniqueness
    bool unique = false;
  };

  /**
   * @brief Table definition.
   *
   * Represents the full structure of a database table,
   * including its columns and indexes.
   */
  struct Table
  {
    /// Table name
    std::string name;

    /// Column definitions
    std::vector<Column> columns;

    /// Index definitions
    std::vector<Index> indexes;

    /**
     * @brief Find a column by name (const).
     *
     * @param n Column name.
     * @return Pointer to the column, or nullptr if not found.
     */
    const Column *findColumn(const std::string &n) const
    {
      for (const auto &c : columns)
        if (c.name == n)
          return &c;
      return nullptr;
    }

    /**
     * @brief Find a column by name.
     *
     * @param n Column name.
     * @return Pointer to the column, or nullptr if not found.
     */
    Column *findColumn(const std::string &n)
    {
      for (auto &c : columns)
        if (c.name == n)
          return &c;
      return nullptr;
    }

    /**
     * @brief Find an index by name (const).
     *
     * @param n Index name.
     * @return Pointer to the index, or nullptr if not found.
     */
    const Index *findIndex(const std::string &n) const
    {
      for (const auto &i : indexes)
        if (i.name == n)
          return &i;
      return nullptr;
    }
  };

  /**
   * @brief Database schema definition.
   *
   * Represents a full database schema as a collection of tables.
   * Schema instances are used as inputs for diffing, migration
   * planning, and serialization.
   */
  struct Schema
  {
    /// Table definitions
    std::vector<Table> tables;

    /**
     * @brief Find a table by name (const).
     *
     * @param n Table name.
     * @return Pointer to the table, or nullptr if not found.
     */
    const Table *findTable(const std::string &n) const
    {
      for (const auto &t : tables)
        if (t.name == n)
          return &t;
      return nullptr;
    }

    /**
     * @brief Find a table by name.
     *
     * @param n Table name.
     * @return Pointer to the table, or nullptr if not found.
     */
    Table *findTable(const std::string &n)
    {
      for (auto &t : tables)
        if (t.name == n)
          return &t;
      return nullptr;
    }
  };

} // namespace vix::db::schema

#endif // VIX_DB_SCHEMA_SCHEMA_HPP
