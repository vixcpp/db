#pragma once
#include "MigratorCLI.hpp"

namespace vix::db::tools
{
  // Uses options: --new, --snapshot, --dir, --name, --dialect
  int run_make_migrations(const MigratorCLI::Options &opt);
}
