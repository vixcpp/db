/**
 *
 *  @file MigrationRunner.hpp
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
#ifndef VIX_DB_MIGRATIONS_RUNNER_HPP
#define VIX_DB_MIGRATIONS_RUNNER_HPP

#include <vix/db/mig/Migration.hpp>

#include <vector>

namespace vix::db
{
  /**
   * @brief Execute code-based database migrations.
   *
   * MigrationsRunner applies a sequence of Migration objects
   * against a database connection. Migrations are executed
   * in the order they are added.
   *
   * This runner is intended for migrations expressed directly
   * in C++ code, as opposed to file-based SQL migrations.
   */
  class MigrationsRunner
  {
    Connection &conn_;
    std::vector<Migration *> migs_;

  public:
    /**
     * @brief Construct a migrations runner.
     *
     * @param c Database connection.
     */
    explicit MigrationsRunner(Connection &c) : conn_(c) {}

    /**
     * @brief Register a migration.
     *
     * The runner does not take ownership of the migration object;
     * the caller is responsible for managing its lifetime.
     *
     * @param m Pointer to a Migration instance.
     */
    void add(Migration *m) { migs_.push_back(m); }

    /**
     * @brief Execute all registered migrations.
     *
     * Migrations are executed sequentially in the order they
     * were added to the runner.
     */
    void runAll();
  };

} // namespace vix::db

#endif // VIX_DB_MIGRATIONS_RUNNER_HPP
