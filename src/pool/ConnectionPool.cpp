/**
 *
 *  @file ConnectionPool.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#include <vix/db/pool/ConnectionPool.hpp>
#include <vix/db/core/Errors.hpp>

namespace vix::db
{
  ConnectionPtr ConnectionPool::acquire()
  {
    std::unique_lock lk(m_);

    while (!idle_.empty())
    {
      auto c = idle_.front();
      idle_.pop();

      if (!c || !c->ping())
      {
        if (total_ > 0)
          --total_;
        continue;
      }

      return c;
    }

    if (total_ < cfg_.max)
    {
      ++total_;
      lk.unlock();

      auto c = factory_();
      if (!c || !c->ping())
      {
        std::lock_guard g(m_);
        if (total_ > 0)
          --total_;
        throw DBError("ConnectionPool: factory returned invalid connection");
      }
      return c;
    }

    cv_.wait(lk, [&]
             { return !idle_.empty(); });

    // on réessaye via le while
    lk.unlock();
    return acquire();
  }

  void ConnectionPool::release(ConnectionPtr c)
  {
    {
      std::lock_guard lk(m_);
      if (c)
        idle_.push(std::move(c));
    }
    cv_.notify_one();
  }

  void ConnectionPool::warmup()
  {
    std::lock_guard lk(m_);

    for (std::size_t i = 0; i < cfg_.min; ++i)
    {
      auto c = factory_();
      if (!c || !c->ping())
        throw DBError("ConnectionPool::warmup: factory returned invalid connection");

      idle_.push(std::move(c));
      ++total_;
    }
  }

} // namespace vix::db
