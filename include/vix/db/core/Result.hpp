/**
 *
 *  @file Result.hpp
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
#ifndef VIX_DB_RESULT_HPP
#define VIX_DB_RESULT_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace vix::db
{
  /**
   * @brief Represents a single row in a database result set.
   *
   * ResultRow provides read-only, index-based access to the values
   * of the current row. Column indexing is zero-based and follows
   * the order defined by the SQL query.
   *
   * Implementations are provided by concrete database drivers.
   */
  struct ResultRow
  {
    virtual ~ResultRow() = default;

    /**
     * @brief Check whether the value at the given column index is NULL.
     *
     * @param i Column index (zero-based).
     * @return true if the column value is SQL NULL.
     */
    virtual bool isNull(std::size_t i) const = 0;

    /**
     * @brief Retrieve the column value as a string.
     *
     * @param i Column index (zero-based).
     * @return Column value converted to std::string.
     */
    virtual std::string getString(std::size_t i) const = 0;

    /**
     * @brief Retrieve the column value as a 64-bit integer.
     *
     * @param i Column index (zero-based).
     * @return Column value converted to std::int64_t.
     */
    virtual std::int64_t getInt64(std::size_t i) const = 0;

    /**
     * @brief Retrieve the column value as a double.
     *
     * @param i Column index (zero-based).
     * @return Column value converted to double.
     */
    virtual double getDouble(std::size_t i) const = 0;

    /**
     * @brief Retrieve a string value or return a default if NULL.
     *
     * @param i Column index (zero-based).
     * @param def Default value if the column is NULL.
     * @return Column value or the provided default.
     */
    std::string getStringOr(std::size_t i, std::string def = {}) const
    {
      return isNull(i) ? std::move(def) : getString(i);
    }

    /**
     * @brief Retrieve an integer value or return a default if NULL.
     *
     * @param i Column index (zero-based).
     * @param def Default value if the column is NULL.
     * @return Column value or the provided default.
     */
    std::int64_t getInt64Or(std::size_t i, std::int64_t def = 0) const
    {
      return isNull(i) ? def : getInt64(i);
    }

    /**
     * @brief Retrieve a floating-point value or return a default if NULL.
     *
     * @param i Column index (zero-based).
     * @param def Default value if the column is NULL.
     * @return Column value or the provided default.
     */
    double getDoubleOr(std::size_t i, double def = 0.0) const
    {
      return isNull(i) ? def : getDouble(i);
    }
  };

  /**
   * @brief Represents a forward-only result set produced by a query.
   *
   * ResultSet acts as an iterator over rows returned by a database query.
   * The lifetime of the returned ResultRow reference is valid until the
   * next call to next().
   *
   * Implementations are provided by concrete database drivers.
   */
  struct ResultSet
  {
    virtual ~ResultSet() = default;

    /**
     * @brief Advance to the next row in the result set.
     *
     * @return true if a new row is available, false if the end is reached.
     */
    virtual bool next() = 0;

    /**
     * @brief Return the number of columns in the result set.
     *
     * @return Column count.
     */
    virtual std::size_t cols() const = 0;

    /**
     * @brief Access the current row.
     *
     * Must only be called after a successful call to next().
     *
     * @return Reference to the current ResultRow.
     */
    virtual const ResultRow &row() const = 0;
  };

} // namespace vix::db

#endif // VIX_DB_RESULT_HPP
