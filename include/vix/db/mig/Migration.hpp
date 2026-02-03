/**
 *
 *  @file Migration.hpp
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
#ifndef VIX_DB_MIGRATION_HPP
#define VIX_DB_MIGRATION_HPP

#include <string>
#include <vix/db/core/Drivers.hpp>

namespace vix::db
{
  /**
   * @brief Base interface for code-based database migrations.
   *
   * A Migration represents a reversible database schema change
   * implemented in C++ code rather than SQL files.
   *
   * Each migration is uniquely identified by an identifier string
   * (typically prefixed with a sortable timestamp) and must provide
   * both forward (up) and backward (down) operations.
   */
  struct Migration
  {
    virtual ~Migration() = default;

    /**
     * @brief Return the unique migration identifier.
     *
     * The identifier is used to order migrations and to track
     * which migrations have already been applied.
     *
     * @return Migration identifier.
     */
    virtual std::string id() const = 0;

    /**
     * @brief Apply the migration.
     *
     * This method performs the schema changes required to move
     * the database to the next version.
     *
     * @param c Database connection.
     */
    virtual void up(Connection &c) = 0;

    /**
     * @brief Revert the migration.
     *
     * This method must undo the effects of the corresponding
     * up() operation.
     *
     * @param c Database connection.
     */
    virtual void down(Connection &c) = 0;
  };

} // namespace vix::db

#endif // VIX_DB_MIGRATION_HPP
