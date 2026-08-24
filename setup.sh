#!/bin/sh
# One-time setup for brackeys-2026-copycat on macOS.
# Run from the project root: ./setup.sh
set -e

echo "== Xcode command line tools =="
if xcode-select -p >/dev/null 2>&1; then
    echo "already installed: $(xcode-select -p)"
else
    xcode-select --install
    echo "Finish the Xcode CLT installer dialog, then re-run this script."
    exit 1
fi

echo "== Homebrew packages =="
if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew not found — install it from https://brew.sh first." >&2
    exit 1
fi
brew list raylib     >/dev/null 2>&1 || brew install raylib
brew list pkg-config >/dev/null 2>&1 || brew install pkg-config
brew list emscripten >/dev/null 2>&1 || brew install emscripten
echo "raylib $(pkg-config --modversion raylib), $(emcc --version | head -1)"

echo "== Git repo =="
if [ ! -d .git ]; then
    git init -b main
fi
git add -A
git diff --cached --quiet || git commit -m "Project skeleton: raylib + C++17, Makefile, docs"

echo "== GitHub (private repo under ZenonPact) =="
if ! command -v gh >/dev/null 2>&1; then
    echo "GitHub CLI not found — 'brew install gh && gh auth login', then re-run." >&2
    exit 1
fi
if git remote get-url origin >/dev/null 2>&1; then
    echo "remote 'origin' already set: $(git remote get-url origin)"
    git push -u origin main
else
    gh repo create ZenonPact/brackeys-2026-copycat --private --source=. --remote=origin --push
fi

echo "== Done. Try: make run =="
