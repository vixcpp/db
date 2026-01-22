/**
 *
 *  @file Errors.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/vixcpp/vix
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_DB_ERRORS_HPP
#define VIX_DB_ERRORS_HPP

#include <string>
#include <stdexcept>

namespace vix::db
{
  struct DBError : std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };

  struct TxnError : DBError
  {
    using DBError::DBError;
  };

  struct NotFound : DBError
  {
    using DBError::DBError;
  };
} // namespace vix::db

#endif // VIX_DB_ERRORS_HPP
