/**
 *
 *  @file SQLiteGenerator.hpp
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
#ifndef VIX_DB_MIG_SQL_SQLITE_GENERATOR_HPP
#define VIX_DB_MIG_SQL_SQLITE_GENERATOR_HPP

#include <vix/db/mig/diff/Op.hpp>

#include <string>
#include <vector>

namespace vix::db::mig::sql
{
  /**
   * @brief Generate SQLite SQL statements for applying a migration.
   *
   * Converts portable migration operations into SQLite DDL. Unsupported
   * SQLite alterations fail with std::runtime_error before any SQL script
   * is returned.
   *
   * @param ops Ordered list of migration operations.
   * @return SQL script for the "up" migration.
   */
  std::string to_sqlite_up(const std::vector<vix::db::mig::diff::Op> &ops);

  /**
   * @brief Generate SQLite SQL statements for reverting a migration.
   *
   * Produces the inverse SQL script in reverse operation order.
   * SQLite DROP COLUMN is emitted using the native SQLite 3.35+ syntax.
   *
   * @param ops Ordered list of migration operations.
   * @return SQL script for the "down" migration.
   */
  std::string to_sqlite_down(const std::vector<vix::db::mig::diff::Op> &ops);

} // namespace vix::db::mig::sql

#endif // VIX_DB_MIG_SQL_SQLITE_GENERATOR_HPP
