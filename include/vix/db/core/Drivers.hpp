/**
 *
 *  @file Drivers.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_DB_DRIVERS_HPP
#define VIX_DB_DRIVERS_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

#include <vix/db/core/Result.hpp>
#include <vix/db/core/Value.hpp>

namespace vix::db
{
  struct Statement
  {
    virtual ~Statement() = default;
    virtual void bind(std::size_t idx, const DbValue &v) = 0;
    void bind(std::size_t idx, std::int64_t v) { bind(idx, i64(v)); }
    void bind(std::size_t idx, double v) { bind(idx, f64(v)); }
    void bind(std::size_t idx, bool v) { bind(idx, b(v)); }
    void bind(std::size_t idx, std::string v) { bind(idx, str(std::move(v))); }
    void bind(std::size_t idx, std::string_view v) { bind(idx, std::string(v)); }
    void bindNull(std::size_t idx) { bind(idx, null()); }
    virtual std::unique_ptr<ResultSet> query() = 0;
    virtual std::uint64_t exec() = 0;
  };

  struct Connection
  {
    virtual ~Connection() = default;
    virtual std::unique_ptr<Statement> prepare(std::string_view sql) = 0;
    virtual void begin() = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
    virtual std::uint64_t lastInsertId() = 0;
    virtual bool ping() { return true; }
  };

  using ConnectionPtr = std::shared_ptr<Connection>;
  using ConnectionFactory = std::function<ConnectionPtr()>;

} // namespace vix::db

#endif // VIX_DB_DRIVERS_HPP
