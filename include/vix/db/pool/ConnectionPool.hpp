/**
 *
 *  @file ConnectionPool.hpp
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
  /**
   * @brief Configuration parameters for a database connection pool.
   */
  struct PoolConfig
  {
    /// Minimum number of connections kept in the pool
    std::size_t min = 1;

    /// Maximum number of connections allowed in the pool
    std::size_t max = 8;
  };

  /**
   * @brief Thread-safe pool of database connections.
   *
   * ConnectionPool manages a set of database connections created
   * using a user-provided ConnectionFactory. Connections are reused
   * across callers to reduce connection overhead and control resource
   * usage.
   *
   * The pool enforces a maximum number of total connections and blocks
   * callers when no connection is available.
   */
  class ConnectionPool
  {
    ConnectionFactory factory_;
    PoolConfig cfg_{};

    std::mutex m_;
    std::condition_variable cv_;
    std::queue<ConnectionPtr> idle_;
    std::size_t total_ = 0;

  public:
    /**
     * @brief Construct a connection pool.
     *
     * @param factory Factory function used to create new connections.
     * @param cfg     Pool configuration parameters.
     */
    ConnectionPool(ConnectionFactory factory, PoolConfig cfg = {})
        : factory_(std::move(factory)), cfg_(cfg) {}

    /**
     * @brief Acquire a connection from the pool.
     *
     * If no idle connection is available and the maximum number of
     * connections has not been reached, a new connection is created.
     * Otherwise, the call blocks until a connection is released.
     *
     * @return Shared pointer to an active database connection.
     */
    ConnectionPtr acquire();

    /**
     * @brief Release a connection back to the pool.
     *
     * @param c Connection to release.
     */
    void release(ConnectionPtr c);

    /**
     * @brief Pre-create the minimum number of connections.
     *
     * Ensures that at least PoolConfig::min connections are created
     * and available in the idle pool.
     */
    void warmup();
  };

  /**
   * @brief RAII wrapper for pooled database connections.
   *
   * PooledConn acquires a connection from a ConnectionPool on
   * construction and automatically releases it back to the pool
   * on destruction.
   *
   * This type is non-copyable but movable.
   */
  class PooledConn final
  {
    ConnectionPool &pool_;
    ConnectionPtr c_;

  public:
    /**
     * @brief Acquire a pooled connection.
     *
     * @param p Reference to the connection pool.
     */
    explicit PooledConn(ConnectionPool &p)
        : pool_(p), c_(p.acquire()) {}

    /**
     * @brief Release the connection back to the pool.
     */
    ~PooledConn() noexcept
    {
      if (c_)
        pool_.release(std::move(c_));
    }

    PooledConn(const PooledConn &) = delete;
    PooledConn &operator=(const PooledConn &) = delete;

    /**
     * @brief Move-construct a pooled connection.
     *
     * @param other Pooled connection to move from.
     */
    PooledConn(PooledConn &&other) noexcept
        : pool_(other.pool_), c_(std::move(other.c_)) {}

    /**
     * @brief Move-assign a pooled connection.
     *
     * Both pooled connections must originate from the same pool.
     *
     * @param other Pooled connection to move from.
     * @return Reference to this instance.
     */
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

    /**
     * @brief Access the underlying connection.
     *
     * @return Reference to the connection.
     */
    Connection &get() { return *c_; }

    /// Pointer-like access to the connection
    Connection *operator->() { return c_.get(); }

    /// Dereference access to the connection
    Connection &operator*() { return *c_; }

    /**
     * @brief Access the underlying shared pointer.
     *
     * @return Reference to the ConnectionPtr.
     */
    ConnectionPtr &ptr() { return c_; }

    /**
     * @brief Access the underlying shared pointer (const).
     *
     * @return Const reference to the ConnectionPtr.
     */
    const ConnectionPtr &ptr() const { return c_; }
  };

} // namespace vix::db

#endif // VIX_DB_CONNECTION_POOL_HPP
