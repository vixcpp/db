

/**
 *
 *  @file db.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */

#ifndef VIX_DB_HPP
#define VIX_DB_HPP

#include <vix/db/Errors.hpp>
#include <vix/db/Drivers.hpp>
#if VIX_DB_HAS_MYSQL
#include <vix/db/MySQLDriver.hpp>
#endif
#include <vix/db/ConnectionPool.hpp>
#include <vix/db/Transaction.hpp>
#include <vix/db/Migration.hpp>
#include <vix/db/MigrationsRunner.hpp>

#endif
