/**
 *
 *  @file Errors.hpp
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
#ifndef VIX_DB_ERRORS_HPP
#define VIX_DB_ERRORS_HPP

#include <stdexcept>
#include <string>

namespace vix::db
{
  /**
   * @brief Base exception type for database-related errors.
   *
   * DBError is the root exception for all errors originating from the
   * database abstraction layer. It derives from std::runtime_error
   * and is intended to be caught either directly or via more specific
   * derived error types.
   */
  struct DBError : std::runtime_error
  {
    /**
     * @brief Construct a database error with a descriptive message.
     *
     * @param message Human-readable error description.
     */
    using std::runtime_error::runtime_error;
  };

  /**
   * @brief Transaction-related error.
   *
   * Thrown when a transaction operation fails, such as:
   * - begin failure
   * - commit failure
   * - rollback failure
   *
   * Drivers or higher-level abstractions may use this to signal
   * transactional inconsistencies or backend errors.
   */
  struct TxnError : DBError
  {
    /**
     * @brief Construct a transaction error.
     *
     * @param message Human-readable error description.
     */
    using DBError::DBError;
  };

  /**
   * @brief Error indicating that a requested entity was not found.
   *
   * Typically used when a query executes successfully but does not
   * return the expected row or resource.
   *
   * Examples:
   * - Missing record by primary key
   * - Empty result where one row was expected
   */
  struct NotFound : DBError
  {
    /**
     * @brief Construct a not-found error.
     *
     * @param message Human-readable error description.
     */
    using DBError::DBError;
  };

} // namespace vix::db

#endif // VIX_DB_ERRORS_HPP
