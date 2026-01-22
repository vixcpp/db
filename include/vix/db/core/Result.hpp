/**
 *
 *  @file Result.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
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
  struct ResultRow
  {
    virtual ~ResultRow() = default;

    virtual bool isNull(std::size_t i) const = 0;
    virtual std::string getString(std::size_t i) const = 0;
    virtual std::int64_t getInt64(std::size_t i) const = 0;
    virtual double getDouble(std::size_t i) const = 0;

    std::string getStringOr(std::size_t i, std::string def = {}) const
    {
      return isNull(i) ? std::move(def) : getString(i);
    }

    std::int64_t getInt64Or(std::size_t i, std::int64_t def = 0) const
    {
      return isNull(i) ? def : getInt64(i);
    }

    double getDoubleOr(std::size_t i, double def = 0.0) const
    {
      return isNull(i) ? def : getDouble(i);
    }
  };

  struct ResultSet
  {
    virtual ~ResultSet() = default;
    virtual bool next() = 0;
    virtual std::size_t cols() const = 0;
    virtual const ResultRow &row() const = 0;
  };

} // namespace vix::db

#endif // VIX_DB_RESULT_HPP
