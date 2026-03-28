# Vix DB

<p align="center">
  <strong>The explicit database layer for modern C++ systems</strong><br/>
  Deterministic · Predictable · No magic
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue" />
  <img src="https://img.shields.io/badge/License-MIT-green" />
  <img src="https://img.shields.io/badge/Status-Active-success" />
</p>

---

## What is Vix DB?

**Vix DB** is the database foundation of **Vix.cpp**.

It gives you a **direct, explicit, and predictable way** to work with databases in C++ without hiding anything.

```cpp
auto db = vix::db::Database::sqlite("vix.db");

auto conn = db.pool().acquire();
conn->prepare("SELECT 1")->exec();
```

No setup. No hidden layers. No surprises.

---

## The problem with existing abstractions

Most database tools today:

- hide SQL behind heavy abstractions
- introduce implicit behavior
- make performance unpredictable
- couple your code to a framework

This creates systems that are:

- harder to debug
- harder to optimize
- harder to maintain over time

---

## The Vix approach

Vix DB does the opposite.

> You control everything. The system stays transparent.

- SQL is explicit
- transactions are explicit
- connections are explicit
- behavior is deterministic

No hidden queries. No implicit state. No magic.

---

## Designed for real systems

Vix DB is built for environments where things actually matter:

- high-load backend services
- edge and offline-first systems
- unreliable networks
- long-running production systems

This aligns directly with the philosophy behind **Vix.cpp** and **Softadastra**:

> Systems must keep working, even when conditions are not ideal.

---

## What you get

Vix DB focuses on **fundamentals done right**:

- connection pooling (thread-safe)
- explicit transactions (RAII)
- prepared statements
- deterministic query execution
- migration system (code + files)
- clean driver abstraction (MySQL, SQLite)
- zero runtime overhead
- zero hidden allocations

Everything is:

> **opt-in · explicit · predictable**

---

## Ultra simple API

All complexity is inside Vix.
Your code stays minimal.

### SQLite

```cpp
auto db = vix::db::Database::sqlite("vix.db");
```

### MySQL

```cpp
auto db = vix::db::Database::mysql(
  "tcp://127.0.0.1:3306",
  "root",
  "",
  "vixdb"
);
```

### Query

```cpp
auto conn = db.pool().acquire();

auto st = conn->prepare("SELECT id, name FROM users WHERE age > ?");
st->bind(1, std::int64_t(18));

auto rs = st->query();
while (rs->next())
{
  const auto &row = rs->row();
  std::cout << row.getInt64(0) << " " << row.getString(1) << "\n";
}
```

---

## Transactions (RAII)

```cpp
vix::db::Transaction tx(db.pool());

tx.conn().prepare("INSERT INTO users (name) VALUES (?)")
    ->bind(1, std::string("Alice"))
    ->exec();

tx.commit();
```

---

## Migrations

Supports both:

- code-based migrations
- file-based migrations

```cpp
vix::db::MigrationsRunner runner(conn);
runner.runAll();
```

---

## ORM is optional

Vix DB is **not an ORM**.

It is the **foundation layer**.

If you want higher-level abstractions, Vix provides an optional ORM module.

If you don’t:

> Vix DB is complete on its own.

No forced abstraction. No lock-in.

---

## Built for modern C++

- C++20
- RAII everywhere
- explicit ownership
- no hidden lifetimes
- clean error handling

If you like **clear and intentional C++**, this will feel natural.

---

## Architecture

```text
Drivers (MySQL / SQLite)
        ↓
Connection Pool
        ↓
Database (Facade)
        ↓
Your code / ORM (optional)
```

Clear layers. No coupling. No surprises.

---

## Part of Vix.cpp

Vix DB is part of **Vix.cpp**, an offline-first, peer-to-peer, high-performance C++ runtime.

It integrates with:

- Vix Core
- Vix CLI
- ORM (optional)
- WebSocket / networking
- future distributed systems (Softadastra)

---

## Getting started

Build with examples:

```bash
cmake -S . -B build -DVIX_DB_BUILD_EXAMPLES=ON -DVIX_DB_USE_SQLITE=ON
cmake --build build
```

Run:

```bash
./build/examples/vix_db_example_sqlite_basic
```

---

## Why Vix DB matters

This is not just another DB wrapper.

It is part of a bigger vision:

> Rebuilding reliable systems for real-world conditions.

- explicit control
- deterministic behavior
- offline-first ready
- production-first mindset

---

## ⭐ Support the project

If you believe:

- C++ deserves modern infrastructure
- systems should be predictable
- databases should not hide behavior

then consider starring the project.

👉 https://github.com/vixcpp/vix

---

MIT License

