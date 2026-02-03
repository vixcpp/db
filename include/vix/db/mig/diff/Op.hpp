/**
 *
 *  @file Op.hpp
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
#ifndef VIX_DB_MIG_DIFF_OP_HPP
#define VIX_DB_MIG_DIFF_OP_HPP

#include <vix/db/schema/Schema.hpp>

#include <string>
#include <variant>
#include <vector>

namespace vix::db::mig::diff
{
  using vix::db::schema::Column;
  using vix::db::schema::Index;
  using vix::db::schema::Table;

  /**
   * @brief Operation representing table creation.
   *
   * Contains the full table definition required to create
   * the table in the target schema.
   */
  struct CreateTable
  {
    /// Table definition to be created
    Table table;
  };

  /**
   * @brief Operation representing table removal.
   *
   * The full table definition is retained to allow
   * reverse migrations (down).
   */
  struct DropTable
  {
    /// Full table definition kept for rollback
    Table table;
  };

  /**
   * @brief Operation representing column addition.
   */
  struct AddColumn
  {
    /// Target table name
    std::string table;

    /// Column definition to add
    Column column;
  };

  /**
   * @brief Operation representing column removal.
   *
   * The full column definition is retained to allow
   * re-adding the column during rollback.
   */
  struct DropColumn
  {
    /// Target table name
    std::string table;

    /// Column definition kept for rollback
    Column column;
  };

  /**
   * @brief Operation representing index creation.
   */
  struct CreateIndex
  {
    /// Target table name
    std::string table;

    /// Index definition to create
    Index index;
  };

  /**
   * @brief Operation representing index removal.
   *
   * The index definition is retained so it can be
   * recreated during rollback.
   */
  struct DropIndex
  {
    /// Target table name
    std::string table;

    /// Index definition kept for rollback
    Index index;
  };

  /**
   * @brief Variant type representing a single migration operation.
   *
   * An Op describes one atomic schema change produced by the
   * migration diff engine. Operations are intended to be applied
   * sequentially in the order produced by the diff.
   *
   * Supported operations:
   * - CreateTable
   * - DropTable
   * - AddColumn
   * - DropColumn
   * - CreateIndex
   * - DropIndex
   */
  using Op = std::variant<
      CreateTable,
      DropTable,
      AddColumn,
      DropColumn,
      CreateIndex,
      DropIndex>;

} // namespace vix::db::mig::diff

#endif // VIX_DB_MIG_DIFF_OP_HPP
