#include <vix/db/db.hpp>
#include <iostream>

int main()
{
  vix::db::DbConfig cfg;
  cfg.engine = vix::db::Engine::MySQL;
  cfg.mysql.host = "tcp://127.0.0.1:3306";
  cfg.mysql.user = "root";
  cfg.mysql.password = "";
  cfg.mysql.database = "vixdb";
  cfg.mysql.pool.min = 1;
  cfg.mysql.pool.max = 8;

  vix::db::Database db(cfg);

  try
  {
    // 2) Transaction RAII
    vix::db::Transaction tx(db.pool());

    // Create table (demo)
    tx.conn().prepare(
                 "CREATE TABLE IF NOT EXISTS users ("
                 "  id BIGINT PRIMARY KEY AUTO_INCREMENT,"
                 "  name VARCHAR(255) NOT NULL,"
                 "  age INT NOT NULL"
                 ")")
        ->exec();

    // Insert 1 row
    {
      auto st = tx.conn().prepare("INSERT INTO users (name, age) VALUES (?, ?)");
      st->bind(1, std::string("Alice"));
      st->bind(2, static_cast<std::int64_t>(20)); // int -> i64
      st->exec();
    }

    // Query
    {
      auto st = tx.conn().prepare("SELECT id, name, age FROM users WHERE age >= ?");
      st->bind(1, static_cast<std::int64_t>(18));
      auto rs = st->query();

      while (rs->next())
      {
        const auto &row = rs->row();
        std::cout
            << row.getInt64(0) << " "
            << row.getString(1) << " "
            << row.getInt64(2) << "\n";
      }
    }

    // 3) Commit
    tx.commit();
    std::cout << "Committed.\n";
  }
  catch (const std::exception &e)
  {
    std::cerr << "DB error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
