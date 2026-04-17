/**
 *
 *  @file OwnedResultSet.hpp
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
#ifndef VIX_DB_OWNED_RESULT_SET_HPP
#define VIX_DB_OWNED_RESULT_SET_HPP

#include <memory>
#include <utility>

#include <vix/db/pool/ConnectionPool.hpp>

namespace vix::db
{
  /**
   * @brief Result set wrapper that keeps a pooled connection alive.
   *
   * Some database drivers keep query state tied to the underlying
   * connection and prepared statement. OwnedResultSet ensures that
   * the pooled connection remains valid for the full lifetime of the
   * wrapped ResultSet.
   *
   * Destruction order is important:
   * - the wrapped ResultSet is destroyed first
   * - the pooled connection is released afterwards
   *
   * This guarantees that any driver-owned query resources are cleaned
   * up before the connection returns to the pool.
   */
  class OwnedResultSet final : public ResultSet
  {
  public:
    /**
     * @brief Construct a result set wrapper with connection ownership.
     *
     * @param pooled Active pooled connection.
     * @param result Wrapped driver result set.
     */
    OwnedResultSet(PooledConn pooled,
                   std::unique_ptr<ResultSet> result)
        : pooled_(std::move(pooled)),
          result_(std::move(result))
    {
    }

    OwnedResultSet(const OwnedResultSet &) = delete;
    OwnedResultSet &operator=(const OwnedResultSet &) = delete;
    OwnedResultSet(OwnedResultSet &&) = default;
    OwnedResultSet &operator=(OwnedResultSet &&) = default;

    /**
     * @brief Advance to the next row.
     *
     * @return true if a row is available, false otherwise.
     */
    bool next() override
    {
      return result_->next();
    }

    /**
     * @brief Return the number of columns in the result set.
     *
     * @return Column count.
     */
    std::size_t cols() const override
    {
      return result_->cols();
    }

    /**
     * @brief Access the current row.
     *
     * @return Reference to the current row.
     */
    const ResultRow &row() const override
    {
      return result_->row();
    }

  private:
    /**
     * @brief Pooled connection kept alive for the result lifetime.
     */
    PooledConn pooled_;

    /**
     * @brief Wrapped driver result set.
     */
    std::unique_ptr<ResultSet> result_;
  };

} // namespace vix::db

#endif // VIX_DB_OWNED_RESULT_SET_HPP
