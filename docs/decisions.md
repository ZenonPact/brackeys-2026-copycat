# Decisions — brackeys-2026-copycat

## 1. Tech stack and concept (2026-08-24)

**Decision:** raylib + C++17, single-file start (`src/main.cpp`), built with
clang++ via a plain Makefile (`pkg-config` for raylib flags).

**Concept:** "Copycat" — Brackeys Game Jam 2026 entry.

**Build target:** Web (emscripten/WASM) as the primary jam deliverable, with a
native macOS build as fallback if the web build gives trouble near the
deadline.

**Rationale:**
- raylib keeps the loop tight for a jam — no engine overhead, instant compile-run cycles.
- Single file until the code demands otherwise; split only when it hurts.
- Web builds get far more jam ratings than downloadable builds; native mac is the safety net.
