#ifndef VIX_DB_VALUE_HPP
#define VIX_DB_VALUE_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace vix::db
{
  struct Blob
  {
    std::vector<std::uint8_t> bytes;
  };

  using DbValue = std::variant<
      std::nullptr_t,
      bool,
      std::int64_t,
      double,
      std::string,
      Blob>;

  inline DbValue null() { return nullptr; }

  // Helpers (ergonomie)
  inline DbValue i64(std::int64_t v) { return v; }
  inline DbValue f64(double v) { return v; }
  inline DbValue b(bool v) { return v; }
  inline DbValue str(std::string v) { return DbValue{std::move(v)}; }
}

#endif
