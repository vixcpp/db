#ifndef VIX_DB_SCHEMA_TYPES_HPP
#define VIX_DB_SCHEMA_TYPES_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace vix::db::schema
{
  enum class Dialect
  {
    MySQL,
    SQLite
  };

  enum class BaseType
  {
    Int,
    BigInt,
    Double,
    Bool,
    VarChar,
    Text,
    DateTime
  };

  struct Type
  {
    BaseType base{};
    std::optional<std::uint32_t> size; // for VARCHAR(n)

    static Type Int() { return {BaseType::Int, std::nullopt}; }
    static Type BigInt() { return {BaseType::BigInt, std::nullopt}; }
    static Type Double() { return {BaseType::Double, std::nullopt}; }
    static Type Bool() { return {BaseType::Bool, std::nullopt}; }
    static Type Text() { return {BaseType::Text, std::nullopt}; }
    static Type DateTime() { return {BaseType::DateTime, std::nullopt}; }

    static Type VarChar(std::uint32_t n) { return {BaseType::VarChar, n}; }

    bool operator==(const Type &o) const { return base == o.base && size == o.size; }
    bool operator!=(const Type &o) const { return !(*this == o); }
  };

  struct DefaultValue
  {
    // Keep as raw SQL literal for MVP ("0", "'text'", "CURRENT_TIMESTAMP")
    std::string sql_literal;
  };

} // namespace vix::db::schema

#endif
