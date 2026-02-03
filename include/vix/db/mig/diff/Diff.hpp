/**
 *
 *  @file diff.hpp
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
#ifndef VIX_DB_MIG_DIFF_DIFF_HPP
#define VIX_DB_MIG_DIFF_DIFF_HPP

#include <vix/db/mig/diff/Op.hpp>
#include <vix/db/schema/Schema.hpp>

#include <vector>

namespace vix::db::mig::diff
{
  /**
   * @brief Compute the migration operations required to transform one schema into another.
   *
   * This function compares two database schemas and produces an ordered list
   * of migration operations (`Op`) that, when applied sequentially, transform
   * the @p from schema into the @p to schema.
   *
   * ### Supported operations (MVP)
   * - Create table
   * - Drop table
   * - Add column
   * - Drop column
   * - Create index
   * - Drop index
   *
   * More advanced features (constraints, foreign keys, column alteration,
   * data migration) may be added in future iterations.
   *
   * @param from Source schema.
   * @param to   Target schema.
   *
   * @return Ordered list of migration operations.
   *
   * @throws std::exception if the schemas are incompatible or the diff
   *         cannot be computed safely.
   */
  std::vector<Op> diff_or_throw(const vix::db::schema::Schema &from,
                                const vix::db::schema::Schema &to);

} // namespace vix::db::mig::diff

#endif // VIX_DB_MIG_DIFF_DIFF_HPP
