/**
 *
 *  @file MigrationsRunner.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#include <vix/db/mig/MigrationsRunner.hpp>

namespace vix::db
{
  void MigrationsRunner::runAll()
  {
    for (auto *m : migs_)
    {
      m->up(conn_);
    }
  }
} // namespace vix::db
