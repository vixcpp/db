/**
 *
 *  @file Types.hpp
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
#ifndef VIX_DB_SCHEMA_TYPES_HPP
#define VIX_DB_SCHEMA_TYPES_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace vix::db::schema
{
  /**
   * @brief Supported SQL dialects.
   *
   * Used by SQL generators to emit dialect-specific syntax.
   */
  enum class Dialect
  {
    /// MySQL-compatible dialect
    MySQL,

    /// SQLite dialect
    SQLite
  };

  /**
   * @brief Base column types supported by the schema system.
   *
   * This set represents a minimal, portable subset of SQL types
   * that can be mapped across supported database engines.
   */
  enum class BaseType
  {
    /// 32-bit integer
    Int,

    /// 64-bit integer
    BigInt,

    /// Double-precision floating point
    Double,

    /// Boolean value
    Bool,

    /// Variable-length character string
    VarChar,

    /// Large text field
    Text,

    /// Date and time value
    DateTime
  };

  /**
   * @brief Column type descriptor.
   *
   * Represents a typed column with optional size information.
   * Used by schema definitions, diffing, and SQL generation.
   */
  struct Type
  {
    /// Base type
    BaseType base{};

    /// Optional size parameter (e.g. VARCHAR(n))
    std::optional<std::uint32_t> size;

    /// Convenience constructors for common types
    static Type Int() { return {BaseType::Int, std::nullopt}; }
    static Type BigInt() { return {BaseType::BigInt, std::nullopt}; }
    static Type Double() { return {BaseType::Double, std::nullopt}; }
    static Type Bool() { return {BaseType::Bool, std::nullopt}; }
    static Type Text() { return {BaseType::Text, std::nullopt}; }
    static Type DateTime() { return {BaseType::DateTime, std::nullopt}; }

    /**
     * @brief Create a VARCHAR type with a size constraint.
     *
     * @param n Maximum number of characters.
     * @return VARCHAR type.
     */
    static Type VarChar(std::uint32_t n) { return {BaseType::VarChar, n}; }

    /**
     * @brief Equality comparison.
     */
    bool operator==(const Type &o) const { return base == o.base && size == o.size; }

    /**
     * @brief Inequality comparison.
     */
    bool operator!=(const Type &o) const { return !(*this == o); }
  };

  /**
   * @brief Default value descriptor.
   *
   * For the MVP, default values are represented as raw SQL literals
   * (e.g. "0", "'text'", "CURRENT_TIMESTAMP") and are emitted directly
   * by SQL generators.
   *
   * Future iterations may introduce a typed representation.
   */
  struct DefaultValue
  {
    /// Raw SQL literal
    std::string sql_literal;
  };

} // namespace vix::db::schema

#endif // VIX_DB_SCHEMA_TYPES_HPP
