/**
 *
 *  @file ConnectionPool.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_DB_CONNECTION_POOL_HPP
#define VIX_DB_CONNECTION_POOL_HPP

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>

#include <vix/db/core/Drivers.hpp>

namespace vix::db
{
  struct PoolConfig
  {
    std::size_t min = 1;
    std::size_t max = 8;
  };

  class ConnectionPool
  {
    ConnectionFactory factory_;
    PoolConfig cfg_{};

    std::mutex m_;
    std::condition_variable cv_;
    std::queue<ConnectionPtr> idle_;
    std::size_t total_ = 0;

  public:
    ConnectionPool(ConnectionFactory factory, PoolConfig cfg = {})
        : factory_(std::move(factory)), cfg_(cfg) {}

    ConnectionPtr acquire();
    void release(ConnectionPtr c);
    void warmup();
  };

  class PooledConn final
  {
    ConnectionPool &pool_;
    ConnectionPtr c_;

  public:
    explicit PooledConn(ConnectionPool &p)
        : pool_(p), c_(p.acquire()) {}

    ~PooledConn() noexcept
    {
      if (c_)
        pool_.release(std::move(c_));
    }

    PooledConn(const PooledConn &) = delete;
    PooledConn &operator=(const PooledConn &) = delete;

    PooledConn(PooledConn &&other) noexcept
        : pool_(other.pool_), c_(std::move(other.c_)) {}

    PooledConn &operator=(PooledConn &&other) noexcept
    {
      if (this == &other)
        return *this;

      if (&pool_ != &other.pool_)
        std::terminate();

      if (c_)
        pool_.release(std::move(c_));

      c_ = std::move(other.c_);
      return *this;
    }

    Connection &get() { return *c_; }
    Connection *operator->() { return c_.get(); }
    Connection &operator*() { return *c_; }

    ConnectionPtr &ptr() { return c_; }
    const ConnectionPtr &ptr() const { return c_; }
  };

} // namespace vix::db

#endif // VIX_DB_CONNECTION_POOL_HPP
