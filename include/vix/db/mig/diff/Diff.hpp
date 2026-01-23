#ifndef VIX_DB_MIG_DIFF_DIFF_HPP
#define VIX_DB_MIG_DIFF_DIFF_HPP

#include <vix/db/mig/diff/Op.hpp>
#include <vix/db/schema/Schema.hpp>

#include <vector>

namespace vix::db::mig::diff
{
  // MVP: only supports create/drop table, add/drop column, create/drop index
  std::vector<Op> diff_or_throw(const vix::db::schema::Schema &from,
                                const vix::db::schema::Schema &to);

} // namespace vix::db::mig::diff

#endif
