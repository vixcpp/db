#include <vix/db/schema/Json.hpp>

#include <nlohmann/json.hpp>
#include <stdexcept>

namespace vix::db::schema
{
  using json = nlohmann::json;

  static std::string base_type_to_string(BaseType t)
  {
    switch (t)
    {
    case BaseType::Int:
      return "int";
    case BaseType::BigInt:
      return "bigint";
    case BaseType::Double:
      return "double";
    case BaseType::Bool:
      return "bool";
    case BaseType::VarChar:
      return "varchar";
    case BaseType::Text:
      return "text";
    case BaseType::DateTime:
      return "datetime";
    }
    return "unknown";
  }

  static BaseType base_type_from_string(const std::string &s)
  {
    if (s == "int")
      return BaseType::Int;
    if (s == "bigint")
      return BaseType::BigInt;
    if (s == "double")
      return BaseType::Double;
    if (s == "bool")
      return BaseType::Bool;
    if (s == "varchar")
      return BaseType::VarChar;
    if (s == "text")
      return BaseType::Text;
    if (s == "datetime")
      return BaseType::DateTime;
    throw std::runtime_error("Unknown base type: " + s);
  }

  static json type_to_json(const Type &t)
  {
    json j;
    j["base"] = base_type_to_string(t.base);
    if (t.size)
      j["size"] = *t.size;
    return j;
  }

  static Type type_from_json(const json &j)
  {
    Type t;
    t.base = base_type_from_string(j.at("base").get<std::string>());
    if (j.contains("size"))
      t.size = j.at("size").get<std::uint32_t>();
    return t;
  }

  static json column_to_json(const Column &c)
  {
    json j;
    j["name"] = c.name;
    j["type"] = type_to_json(c.type);
    j["nullable"] = c.nullable;
    j["primary_key"] = c.primary_key;
    j["auto_increment"] = c.auto_increment;
    j["unique"] = c.unique;
    if (c.def)
      j["default"] = c.def->sql_literal;
    return j;
  }

  static Column column_from_json(const json &j)
  {
    Column c;
    c.name = j.at("name").get<std::string>();
    c.type = type_from_json(j.at("type"));
    c.nullable = j.value("nullable", true);
    c.primary_key = j.value("primary_key", false);
    c.auto_increment = j.value("auto_increment", false);
    c.unique = j.value("unique", false);
    if (j.contains("default"))
      c.def = DefaultValue{j.at("default").get<std::string>()};
    return c;
  }

  static json index_to_json(const Index &i)
  {
    json j;
    j["name"] = i.name;
    j["columns"] = i.columns;
    j["unique"] = i.unique;
    return j;
  }

  static Index index_from_json(const json &j)
  {
    Index i;
    i.name = j.at("name").get<std::string>();
    i.columns = j.at("columns").get<std::vector<std::string>>();
    i.unique = j.value("unique", false);
    return i;
  }

  static json table_to_json(const Table &t)
  {
    json j;
    j["name"] = t.name;
    j["columns"] = json::array();
    for (const auto &c : t.columns)
      j["columns"].push_back(column_to_json(c));

    j["indexes"] = json::array();
    for (const auto &i : t.indexes)
      j["indexes"].push_back(index_to_json(i));
    return j;
  }

  static Table table_from_json(const json &j)
  {
    Table t;
    t.name = j.at("name").get<std::string>();
    for (const auto &cj : j.at("columns"))
      t.columns.push_back(column_from_json(cj));
    if (j.contains("indexes"))
      for (const auto &ij : j.at("indexes"))
        t.indexes.push_back(index_from_json(ij));
    return t;
  }

  std::string to_json_string(const Schema &s, bool pretty)
  {
    json j;
    j["version"] = 1;
    j["tables"] = json::array();
    for (const auto &t : s.tables)
      j["tables"].push_back(table_to_json(t));
    return pretty ? j.dump(2) : j.dump();
  }

  Schema from_json_string_or_throw(const std::string &text)
  {
    auto j = json::parse(text);
    if (!j.contains("tables"))
      throw std::runtime_error("Invalid schema snapshot: missing 'tables'");

    Schema s;
    for (const auto &tj : j.at("tables"))
      s.tables.push_back(table_from_json(tj));
    return s;
  }

} // namespace vix::db::schema
