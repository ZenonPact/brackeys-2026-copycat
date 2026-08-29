# Copycat — Brackeys Game Jam 2026
# Native macOS build via clang++ + raylib (pkg-config).

CXX      := clang++
CXXFLAGS = -std=c++17 -Wall -Wextra $(shell pkg-config --cflags raylib | sed 's/-I/-isystem /g')
LDFLAGS  = $(shell pkg-config --libs raylib)

SRC := src/main.cpp
BIN := build/copycat

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

run: $(BIN)
	./$(BIN)

# --- Web build (emscripten, for itch.io) ---
# Homebrew emcc needs Python >= 3.10; ~/.zshrc exports this, but make may
# run without it (non-login shells, editors), so default it here too.
EMSDK_PYTHON ?= /opt/homebrew/opt/python@3.14/bin/python3.14
export EMSDK_PYTHON

RAYLIB_TAG := 6.0
RAYLIB_DIR := vendor/raylib
RAYLIB_WEB := $(RAYLIB_DIR)/src/libraylib.web.a
WEB_DIR    := build/web
WEB_ZIP    := build/copycat-web.zip
ASSETS     := $(wildcard assets/*)

$(RAYLIB_DIR)/src/Makefile:
	git clone --depth 1 --branch $(RAYLIB_TAG) https://github.com/raysan5/raylib.git $(RAYLIB_DIR)

$(RAYLIB_WEB): $(RAYLIB_DIR)/src/Makefile
	$(MAKE) -C $(RAYLIB_DIR)/src PLATFORM=PLATFORM_WEB RAYLIB_RELEASE_PATH=.

$(WEB_DIR)/index.html: $(SRC) $(RAYLIB_WEB) $(ASSETS)
	@mkdir -p $(WEB_DIR)
	em++ $(SRC) -o $@ \
		-std=c++17 -Os -Wall -Wextra \
		-isystem $(RAYLIB_DIR)/src -DPLATFORM_WEB \
		$(RAYLIB_WEB) \
		-sUSE_GLFW=3 -sASYNCIFY -sALLOW_MEMORY_GROWTH=1 \
		--preload-file assets \
		--shell-file $(RAYLIB_DIR)/src/minshell.html

web: $(WEB_DIR)/index.html
	cd $(WEB_DIR) && zip -X -r ../$(notdir $(WEB_ZIP)) index.html index.js index.wasm index.data
	@echo "Ready for itch: $(WEB_ZIP)"

clean:
	rm -rf build

.PHONY: all run web clean
