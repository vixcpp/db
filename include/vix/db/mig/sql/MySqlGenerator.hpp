#ifndef VIX_DB_MIG_SQL_MYSQL_GENERATOR_HPP
#define VIX_DB_MIG_SQL_MYSQL_GENERATOR_HPP

#include <vix/db/mig/diff/Op.hpp>

#include <string>
#include <vector>

namespace vix::db::mig::sql
{
  std::string to_mysql_up(const std::vector<vix::db::mig::diff::Op> &ops);
  std::string to_mysql_down(const std::vector<vix::db::mig::diff::Op> &ops);

} // namespace vix::db::mig::sql

#endif
