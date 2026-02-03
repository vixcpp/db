/**
 *
 *  @file Json.hpp
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
#ifndef VIX_DB_SCHEMA_JSON_HPP
#define VIX_DB_SCHEMA_JSON_HPP

#include <vix/db/schema/Schema.hpp>

#include <string>

namespace vix::db::schema
{
  /**
   * @brief Serialize a database schema to a JSON string.
   *
   * Converts a Schema object into a JSON representation.
   * This is typically used to persist schema snapshots,
   * compare schema versions, or generate migration diffs.
   *
   * The JSON format is stable and intended for both
   * machine processing and human inspection.
   *
   * @param s      Schema to serialize.
   * @param pretty Whether to format the JSON output for readability.
   * @return JSON string representation of the schema.
   */
  std::string to_json_string(const Schema &s, bool pretty = true);

  /**
   * @brief Deserialize a database schema from a JSON string.
   *
   * Parses a JSON schema snapshot and reconstructs the corresponding
   * Schema object.
   *
   * @param json JSON string containing a schema representation.
   * @return Reconstructed Schema instance.
   *
   * @throws std::exception if the JSON is invalid or does not represent
   *         a valid schema.
   */
  Schema from_json_string_or_throw(const std::string &json);

} // namespace vix::db::schema

#endif // VIX_DB_SCHEMA_JSON_HPP
