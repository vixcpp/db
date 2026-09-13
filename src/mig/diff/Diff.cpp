#include <vix/db/mig/diff/Diff.hpp>

#include <stdexcept>
#include <unordered_map>

namespace vix::db::mig::diff
{
  using vix::db::schema::Schema;
  using vix::db::schema::Table;

  static std::unordered_map<std::string, const Table *> map_tables(const Schema &s)
  {
    std::unordered_map<std::string, const Table *> m;
    m.reserve(s.tables.size());
    for (const auto &t : s.tables)
      m.emplace(t.name, &t);
    return m;
  }

  std::vector<Op> diff_or_throw(const Schema &from, const Schema &to)
  {
    std::vector<Op> ops;

    auto A = map_tables(from);
    auto B = map_tables(to);

    // 1) Drop tables missing in 'to', preserving source schema order.
    for (const auto &table : from.tables)
    {
      if (!B.count(table.name))
        ops.push_back(DropTable{table});
    }

    // 2) Create tables new in 'to', preserving target schema order.
    for (const auto &targetTable : to.tables)
    {
      const auto &name = targetTable.name;
      if (!A.count(name))
      {
        ops.push_back(CreateTable{targetTable});
        continue;
      }

      // 3) Same table: diff columns + indexes
      const auto *oldT = A.at(name);
      const auto *newT = &targetTable;

      // Columns: drops
      for (const auto &c_old : oldT->columns)
      {
        if (!newT->findColumn(c_old.name))
          ops.push_back(DropColumn{name, c_old});
      }

      // Columns: adds
      for (const auto &c_new : targetTable.columns)
      {
        if (!oldT->findColumn(c_new.name))
          ops.push_back(AddColumn{name, c_new});
      }

      // Indexes: drops
      for (const auto &i_old : oldT->indexes)
      {
        if (!newT->findIndex(i_old.name))
          ops.push_back(DropIndex{name, i_old});
      }

      // Indexes: adds
      for (const auto &i_new : targetTable.indexes)
      {
        if (!oldT->findIndex(i_new.name))
          ops.push_back(CreateIndex{name, i_new});
      }
    }

    return ops;
  }

} // namespace vix::db::mig::diff
