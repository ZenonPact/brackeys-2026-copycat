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

clean:
	rm -rf build

.PHONY: all run clean
