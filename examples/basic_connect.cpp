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

  Database db(cfg);

  auto conn = db.pool().acquire();
  if (!conn->ping())
  {
    std::cerr << "DB ping failed\n";
    return 1;
  }

  std::cout << "DB connected successfully\n";
  return 0;
}
