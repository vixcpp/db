# Vix.cpp DB Module

Explicit database layer for Vix.cpp.

The DB module provides a predictable C++ database foundation with connection pooling, prepared statements, explicit transactions, migrations, SQLite support, MySQL support, and integration with Vix applications.

## Documentation

Full documentation is available here:

https://docs.vixcpp.com/book/10-database

Database example:

https://docs.vixcpp.com/examples/database

## What DB provides

- SQLite support
- MySQL support
- Connection pooling
- Prepared statements
- Parameter binding
- Query result handling
- Explicit transactions
- RAII connection management
- Code-based migrations
- File-based migrations
- Configuration through `.env`
- Integration with `vix::App`

## Public header

```cpp
#include <vix/db.hpp>
```

For a full Vix application:

```cpp
#include <vix.hpp>
#include <vix/db.hpp>
```

## SQLite example

```cpp
#include <vix/db.hpp>

int main()
{
  auto db = vix::db::Database::sqlite("vix.db");

  db.exec(
      "CREATE TABLE IF NOT EXISTS users ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "name TEXT NOT NULL)");

  db.exec("INSERT INTO users (name) VALUES (?)", "Alice");

  auto rows = db.query("SELECT id, name FROM users");

  while (rows->next())
  {
    const auto &row = rows->row();

    const auto id = row.getInt64(0);
    const auto name = row.getString(1);

    (void)id;
    (void)name;
  }

  return 0;
}
```

Run with SQLite support:

```bash
vix run main.cpp --with-sqlite
```

## MySQL example

```cpp
#include <vix/db.hpp>

int main()
{
  auto db = vix::db::Database::mysql(
      "tcp://127.0.0.1:3306",
      "root",
      "",
      "vixdb");

  db.exec(
      "CREATE TABLE IF NOT EXISTS users ("
      "id BIGINT PRIMARY KEY AUTO_INCREMENT, "
      "name VARCHAR(255) NOT NULL)");

  return 0;
}
```

Run with MySQL support:

```bash
vix run main.cpp --with-mysql
```

## Prepared statements

Use prepared statements for user input:

```cpp
vix::db::PooledConn conn(db.pool());

auto stmt = conn->prepare(
    "SELECT id, name FROM users WHERE id = ?");

stmt->bind(1, static_cast<std::int64_t>(1));

auto rows = stmt->query();
```

## Transactions

```cpp
db.transaction([&](vix::db::Connection &conn)
{
  conn.prepare("INSERT INTO users (name) VALUES (?)")
      ->bind(1, "Alice")
      ->exec();

  conn.prepare("INSERT INTO users (name) VALUES (?)")
      ->bind(1, "Bob")
      ->exec();
});
```

## Configuration

Example `.env`:

```dotenv
DATABASE_ENGINE=sqlite
DATABASE_DEFAULT_NAME=vix.db
SERVER_PORT=8080
```

Use it from C++:

```cpp
vix::config::Config cfg{".env"};
vix::db::Database db{cfg};
```

## Build

Contributors should use the Vix CLI to build this module.

Vix wraps the C++ build workflow with project detection, presets, Ninja builds, clean logs, caching, and focused diagnostics. This keeps the contributor workflow consistent and helps avoid hidden C++ build issues.

### Build with SQLite

```bash
git clone https://github.com/vixcpp/vix.git
cd vix
vix build --with-sqlite
```

### Build with MySQL

```bash
vix build --with-mysql
```

### Build all targets

Use this before running the full test suite, install workflows, or release checks:

```bash
vix build --build-target all
```

### Clean rebuild

Use this when the local CMake cache or build directory may be stale:

```bash
vix build --clean
```

### Release build

```bash
vix build --preset release
```

## Tests

Build all targets first, then run tests:

```bash
vix build --build-target all
vix tests
```

Before opening a pull request, use:

```bash
vix fmt --check
vix build --build-target all
vix tests
```

## Useful links

- Database guide: https://docs.vixcpp.com/book/10-database
- Database example: https://docs.vixcpp.com/examples/database
- Build command: https://docs.vixcpp.com/cli/build
- Tests command: https://docs.vixcpp.com/cli/tests
- Documentation: https://docs.vixcpp.com/
- Engineering notes: https://blog.vixcpp.com/
- Registry: https://registry.vixcpp.com/
- GitHub: https://github.com/vixcpp/vix

## License

MIT License.

See [`LICENSE`](../../LICENSE) for details.
