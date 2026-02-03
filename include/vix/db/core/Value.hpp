/**
 *
 *  @file Value.hpp
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
#ifndef VIX_DB_VALUE_HPP
#define VIX_DB_VALUE_HPP

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace vix::db
{
  /**
   * @brief Binary large object (BLOB) value.
   *
   * Represents raw binary data stored in the database.
   * The interpretation of the data is driver- and application-defined.
   */
  struct Blob
  {
    /// Raw byte buffer
    std::vector<std::uint8_t> bytes;
  };

  /**
   * @brief Type-erased database value.
   *
   * DbValue represents a generic SQL value used for parameter binding
   * and value transport across database drivers.
   *
   * Supported types:
   * - nullptr        : SQL NULL
   * - bool           : Boolean values
   * - std::int64_t   : Integer values
   * - double         : Floating-point values
   * - std::string    : Text values (UTF-8)
   * - Blob           : Binary values
   *
   * This abstraction allows drivers to remain minimal while providing
   * a consistent interface across different database backends.
   */
  using DbValue = std::variant<
      std::nullptr_t,
      bool,
      std::int64_t,
      double,
      std::string,
      Blob>;

  /**
   * @brief Create a SQL NULL value.
   *
   * @return DbValue representing NULL.
   */
  inline DbValue null() { return nullptr; }

  /**
   * @brief Create a 64-bit integer database value.
   *
   * @param v Integer value.
   * @return DbValue wrapping the integer.
   */
  inline DbValue i64(std::int64_t v) { return v; }

  /**
   * @brief Create a floating-point database value.
   *
   * @param v Double value.
   * @return DbValue wrapping the double.
   */
  inline DbValue f64(double v) { return v; }

  /**
   * @brief Create a boolean database value.
   *
   * @param v Boolean value.
   * @return DbValue wrapping the boolean.
   */
  inline DbValue b(bool v) { return v; }

  /**
   * @brief Create a string database value.
   *
   * The string is assumed to be UTF-8 encoded.
   *
   * @param v String value.
   * @return DbValue wrapping the string.
   */
  inline DbValue str(std::string v) { return DbValue{std::move(v)}; }

  /**
   * @brief Create a binary (BLOB) database value.
   *
   * @param bytes Raw byte buffer.
   * @return DbValue wrapping the binary data.
   */
  inline DbValue blob(std::vector<std::uint8_t> bytes)
  {
    return DbValue{Blob{std::move(bytes)}};
  }

} // namespace vix::db

#endif // VIX_DB_VALUE_HPP
