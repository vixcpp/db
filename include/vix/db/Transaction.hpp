/**
 *
 *  @file Transaction.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_DB_TRANSACTION_HPP
#define VIX_DB_TRANSACTION_HPP

#include <vix/db/pool/ConnectionPool.hpp>

namespace vix::db
{
  class Transaction
  {
    PooledConn pooled_;
    bool active_ = true;

  public:
    explicit Transaction(ConnectionPool &pool)
        : pooled_(pool)
    {
      pooled_.get().begin();
    }

    ~Transaction() noexcept
    {
      if (!active_)
        return;
      try
      {
        pooled_.get().rollback();
      }
      catch (...)
      {
      }
    }

    Transaction(const Transaction &) = delete;
    Transaction &operator=(const Transaction &) = delete;

    Transaction(Transaction &&other) noexcept
        : pooled_(std::move(other.pooled_)), active_(other.active_)
    {
      other.active_ = false;
    }

    Transaction &operator=(Transaction &&) = delete;

    void commit()
    {
      pooled_.get().commit();
      active_ = false;
    }

    void rollback()
    {
      pooled_.get().rollback();
      active_ = false;
    }

    Connection &conn() { return pooled_.get(); }
  };

} // namespace vix::db

#endif // VIX_DB_TRANSACTION_HPP
