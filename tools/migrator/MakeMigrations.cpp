#include "MakeMigrations.hpp"

#include <vix/db/mig/diff/Diff.hpp>
#include <vix/db/mig/sql/MySqlGenerator.hpp>
#include <vix/db/schema/Json.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <stdexcept>

namespace vix::db::tools
{
  namespace fs = std::filesystem;
  using vix::db::schema::Schema;

  static std::string read_text(const fs::path &p)
  {
    std::ifstream in(p);
    if (!in)
      throw std::runtime_error("Cannot open file: " + p.string());
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
  }

  static void write_text(const fs::path &p, const std::string &content)
  {
    if (p.has_parent_path())
      fs::create_directories(p.parent_path());
    std::ofstream out(p);
    if (!out)
      throw std::runtime_error("Cannot write file: " + p.string());
    out << content;
  }

  static Schema load_snapshot_or_empty(const fs::path &p)
  {
    if (!fs::exists(p))
      return Schema{};
    return vix::db::schema::from_json_string_or_throw(read_text(p));
  }

  static std::string timestamp_id()
  {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const std::time_t t = system_clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    // stable + sortable
    oss << std::put_time(&tm, "%Y_%m_%d_%H%M%S");
    return oss.str();
  }

  static std::string sanitize(std::string s)
  {
    for (auto &ch : s)
    {
      const bool ok =
          (ch >= 'a' && ch <= 'z') ||
          (ch >= 'A' && ch <= 'Z') ||
          (ch >= '0' && ch <= '9') ||
          ch == '_' || ch == '-';
      if (!ok)
        ch = '_';
    }
    return s.empty() ? std::string("auto") : s;
  }

  int run_make_migrations(const MigratorCLI::Options &opt)
  {
    const fs::path mig_dir = opt.migrationsDir;
    const fs::path snapshot_path = opt.snapshotPath;
    const fs::path new_schema_path = opt.newSchemaPath;

    const Schema oldS = load_snapshot_or_empty(snapshot_path);
    const Schema newS = vix::db::schema::from_json_string_or_throw(read_text(new_schema_path));

    auto ops = vix::db::mig::diff::diff_or_throw(oldS, newS);

    // Always write snapshot (so formatting/version stays stable)
    write_text(snapshot_path, vix::db::schema::to_json_string(newS, true));

    if (ops.empty())
      return 0;

    if (opt.dialect != "mysql")
      throw std::runtime_error("Only --dialect mysql is implemented for now");

    const std::string up_sql = vix::db::mig::sql::to_mysql_up(ops);
    const std::string down_sql = vix::db::mig::sql::to_mysql_down(ops);

    const std::string id = timestamp_id();
    const std::string label = sanitize(opt.name);

    const fs::path up_path = mig_dir / (id + "_" + label + ".up.sql");
    const fs::path down_path = mig_dir / (id + "_" + label + ".down.sql");

    write_text(up_path, up_sql);
    write_text(down_path, down_sql);

    return 0;
  }

} // namespace vix::db::tools
