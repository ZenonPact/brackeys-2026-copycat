# Copycat

Brackeys Game Jam 2026 entry. raylib + C++17, single-file start.

## Build

```sh
make        # build → build/copycat
make run    # build and run
make clean
```

Requires: Xcode command line tools, raylib and pkg-config (via Homebrew).
First-time setup: `./setup.sh` checks and installs everything.

## Layout

- `src/main.cpp` — the game (single file until it hurts)
- `assets/` — art, sound, fonts
- `docs/decisions.md` — why things are the way they are
- `docs/jam-plan.md` — milestones and timeboxing

## Targets

Primary: web build (emscripten) for itch.io. Fallback: native macOS.
