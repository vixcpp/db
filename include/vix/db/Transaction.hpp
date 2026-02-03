/**
 *
 *  @file Transaction.hpp
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
#ifndef VIX_DB_TRANSACTION_HPP
#define VIX_DB_TRANSACTION_HPP

#include <vix/db/pool/ConnectionPool.hpp>

namespace vix::db
{
  /**
   * @brief RAII wrapper for database transactions.
   *
   * Transaction acquires a connection from a ConnectionPool and
   * starts a database transaction on construction. If the transaction
   * is still active when the object is destroyed, it is automatically
   * rolled back.
   *
   * This ensures strong exception safety and prevents leaked
   * transactions.
   */
  class Transaction
  {
    PooledConn pooled_;
    bool active_ = true;

  public:
    /**
     * @brief Begin a new transaction using a pooled connection.
     *
     * @param pool Connection pool from which to acquire a connection.
     */
    explicit Transaction(ConnectionPool &pool)
        : pooled_(pool)
    {
      pooled_.get().begin();
    }

    /**
     * @brief Roll back the transaction if still active.
     *
     * The destructor never throws. Any exception raised during rollback
     * is swallowed to preserve stack unwinding.
     */
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

    /**
     * @brief Move-construct a transaction.
     *
     * Ownership of the active transaction is transferred to the
     * new instance. The source transaction becomes inactive.
     *
     * @param other Transaction to move from.
     */
    Transaction(Transaction &&other) noexcept
        : pooled_(std::move(other.pooled_)), active_(other.active_)
    {
      other.active_ = false;
    }

    Transaction &operator=(Transaction &&) = delete;

    /**
     * @brief Commit the transaction.
     *
     * After commit(), the transaction becomes inactive and will
     * not be rolled back on destruction.
     */
    void commit()
    {
      pooled_.get().commit();
      active_ = false;
    }

    /**
     * @brief Roll back the transaction explicitly.
     *
     * After rollback(), the transaction becomes inactive.
     */
    void rollback()
    {
      pooled_.get().rollback();
      active_ = false;
    }

    /**
     * @brief Access the underlying database connection.
     *
     * @return Reference to the active connection.
     */
    Connection &conn() { return pooled_.get(); }
  };

} // namespace vix::db

#endif // VIX_DB_TRANSACTION_HPP
