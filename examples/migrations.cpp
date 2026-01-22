#include <vix/db/db.hpp>

#include <iostream>
#include <filesystem>
#include <string>

using namespace vix::db;

static DbConfig make_mysql_cfg()
{
  DbConfig cfg;
  cfg.engine = Engine::MySQL;
  cfg.mysql.host = "tcp://127.0.0.1:3306";
  cfg.mysql.user = "root";
  cfg.mysql.password = "";
  cfg.mysql.database = "vixdb";
  cfg.mysql.pool.min = 1;
  cfg.mysql.pool.max = 4;
  return cfg;
}

// ------------------------------
// 1) Code-based migration example
// ------------------------------
class CreateUsersTable final : public Migration
{
public:
  std::string id() const override { return "2026-01-22-create-users"; }

  void up(Connection &c) override
  {
    auto st = c.prepare(
        "CREATE TABLE IF NOT EXISTS users ("
        "  id BIGINT PRIMARY KEY AUTO_INCREMENT,"
        "  name VARCHAR(255) NOT NULL,"
        "  age INT NOT NULL"
        ");");
    st->exec();
  }

  void down(Connection &c) override
  {
    auto st = c.prepare("DROP TABLE IF EXISTS users;");
    st->exec();
  }
};

static void run_code_migrations(Database &db)
{
  std::cout << "[migrations] running code migrations...\n";

  Transaction tx(db.pool());

  CreateUsersTable m1;
  MigrationsRunner runner(tx.conn());
  runner.add(&m1);
  runner.runAll();

  tx.commit();
  std::cout << "[migrations] done (code)\n";
}

// ------------------------------
// 2) File-based migration example
// ------------------------------
static void run_file_migrations(Database &db, std::filesystem::path dir)
{
  std::cout << "[migrations] running file migrations from: " << dir.string() << "\n";

  Transaction tx(db.pool());

  FileMigrationsRunner runner(tx.conn(), std::move(dir));
  runner.setTable("schema_migrations"); // optional (default already)
  runner.applyAll();

  tx.commit();
  std::cout << "[migrations] done (files)\n";
}

int main()
{
  try
  {
    Database db(make_mysql_cfg());

    // 1) Code migrations
    run_code_migrations(db);

    // 2) File migrations (expects ./migrations/*.up.sql and *.down.sql)
    run_file_migrations(db, std::filesystem::path{"migrations"});

    std::cout << "OK\n";
    return 0;
  }
  catch (const std::exception &e)
  {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 1;
  }
}
