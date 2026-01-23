#ifndef VIX_DB_SCHEMA_JSON_HPP
#define VIX_DB_SCHEMA_JSON_HPP

#include <vix/db/schema/Schema.hpp>

#include <string>

namespace vix::db::schema
{
  // Serialize/deserialize schema snapshot (nlohmann/json)
  std::string to_json_string(const Schema &s, bool pretty = true);
  Schema from_json_string_or_throw(const std::string &json);

} // namespace vix::db::schema

#endif
