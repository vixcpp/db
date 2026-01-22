# Vix DB

<p align="center">
  <strong>Low-level database layer for serious C++ systems</strong><br/>
  Deterministic · Explicit · No magic
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue" />
  <img src="https://img.shields.io/badge/License-MIT-green" />
  <img src="https://img.shields.io/badge/Status-Active-success" />
</p>

---

## What is Vix DB?

**Vix DB** is the database foundation of **Vix.cpp**.

It provides a **minimal, explicit, and predictable** interface to databases,
designed for developers who care about:

- performance
- control
- correctness
- long-term maintainability

Vix DB is **not an ORM**.
It is the layer that makes **reliable data access possible** in real systems.

---

## Why a dedicated DB module?

Most database abstractions today:

- hide SQL behind heavy layers
- introduce unpredictable behavior
- make performance harder to reason about
- tightly couple your code to a framework

Vix DB takes the opposite approach.

> **No magic. No hidden queries. No surprises.**

You decide:
- what SQL runs
- when transactions start and end
- how connections are pooled
- how migrations are applied

---

## Designed for real-world conditions

Vix DB is built with the same philosophy as the rest of Vix.cpp:

- systems that must run **under load**
- environments with **limited resources**
- deployments where **debuggability matters**
- teams that want **long-term stability**

This makes it suitable for:
- backend services
- edge systems
- offline-first applications
- P2P or distributed runtimes
- performance-critical APIs

---

## What you get

Vix DB focuses on **fundamentals done right**:

- connection pooling
- explicit transactions
- deterministic query execution
- migrations support
- clean driver boundaries
- zero runtime reflection
- zero garbage collection

Everything is **opt-in** and **pay-for-what-you-use**.

---

## ORM is optional — by design

If you like higher-level abstractions, Vix.cpp provides an optional ORM layer.

If you don’t, **Vix DB works perfectly on its own**.

This separation ensures:
- no forced abstractions
- no performance tax
- no lock-in

You choose the level of abstraction — not the framework.

---

## Built for modern C++

- C++20
- explicit ownership
- RAII-based resource management
- predictable lifetimes
- clean error handling

If you enjoy writing **clear, intentional C++**, Vix DB will feel natural.

---

## Part of the Vix.cpp ecosystem

Vix DB is a core module of **Vix.cpp**, the offline-first, peer-to-peer,
ultra-fast C++ runtime.

It integrates seamlessly with:
- Vix Core
- Vix CLI
- optional ORM layer
- WebSocket & network modules

---

## Getting started

Vix DB is built automatically when enabled through the Vix.cpp umbrella.

See the main repository for installation, CLI usage, and examples.

👉 https://github.com/vixcpp/vix

---

## ⭐ Support the project

If you believe C++ deserves modern, serious infrastructure —
and databases deserve explicit control —

please consider starring **Vix.cpp**.

MIT License
