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

  struct CreateTable
  {
    Table table;
  };

  struct DropTable
  {
    Table table; // keep full table for down
  };

  struct AddColumn
  {
    std::string table;
    Column column;
  };

  struct DropColumn
  {
    std::string table;
    Column column; // keep full def so down can re-add
  };

  struct CreateIndex
  {
    std::string table;
    Index index;
  };

  struct DropIndex
  {
    std::string table;
    Index index; // keep def for down (recreate)
  };

  using Op = std::variant<CreateTable, DropTable, AddColumn, DropColumn, CreateIndex, DropIndex>;

} // namespace vix::db::mig::diff

#endif
