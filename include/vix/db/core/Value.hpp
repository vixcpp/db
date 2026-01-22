/**
 *
 *  @file Value.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
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
  struct Blob
  {
    std::vector<std::uint8_t> bytes;
  };

  using DbValue = std::variant<
      std::nullptr_t,
      bool,
      std::int64_t,
      double,
      std::string,
      Blob>;

  inline DbValue null() { return nullptr; }

  inline DbValue i64(std::int64_t v) { return v; }
  inline DbValue f64(double v) { return v; }
  inline DbValue b(bool v) { return v; }
  inline DbValue str(std::string v) { return DbValue{std::move(v)}; }

  inline DbValue blob(std::vector<std::uint8_t> bytes)
  {
    return DbValue{Blob{std::move(bytes)}};
  }
} // namespace vix::db

#endif
