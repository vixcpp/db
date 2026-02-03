/**
 *
 *  @file MySqlGenerator.hpp
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
#ifndef VIX_DB_MIG_SQL_MYSQL_GENERATOR_HPP
#define VIX_DB_MIG_SQL_MYSQL_GENERATOR_HPP

#include <vix/db/mig/diff/Op.hpp>

#include <string>
#include <vector>

namespace vix::db::mig::sql
{
  /**
   * @brief Generate MySQL SQL statements for applying a migration.
   *
   * Converts a sequence of migration operations into an SQL script
   * suitable for applying the migration on a MySQL-compatible database.
   *
   * The operations are expected to be ordered as produced by the
   * migration diff engine.
   *
   * @param ops Ordered list of migration operations.
   * @return SQL script for the "up" migration.
   */
  std::string to_mysql_up(const std::vector<vix::db::mig::diff::Op> &ops);

  /**
   * @brief Generate MySQL SQL statements for reverting a migration.
   *
   * Produces an SQL script that reverts the effects of the provided
   * migration operations. The generator relies on the preserved
   * operation metadata to safely reconstruct the previous schema.
   *
   * @param ops Ordered list of migration operations.
   * @return SQL script for the "down" migration.
   */
  std::string to_mysql_down(const std::vector<vix::db::mig::diff::Op> &ops);

} // namespace vix::db::mig::sql

#endif // VIX_DB_MIG_SQL_MYSQL_GENERATOR_HPP
