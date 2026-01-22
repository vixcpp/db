#include <vix/db/db.hpp>
#include <iostream>

using namespace vix::db;

int main()
{
  DbConfig cfg;
  cfg.engine = Engine::MySQL;
  cfg.mysql.host = "tcp://127.0.0.1:3306";
  cfg.mysql.user = "root";
  cfg.mysql.password = "";
  cfg.mysql.database = "vixdb";
  cfg.mysql.pool.min = 1;
  cfg.mysql.pool.max = 8;

  Database db(cfg);

  auto conn = db.pool().acquire();
  auto st = conn->prepare("SELECT id, name FROM users WHERE age > ?");

  st->bind(1, 18);
  // or: st->bind(1, std::int64_t{18});
  // or: st->bind(1, i64(18));

  auto rs = st->query();
  while (rs->next())
  {
    const auto &row = rs->row();
    std::cout << row.getInt64(0) << " " << row.getString(1) << "\n";
  }
}
