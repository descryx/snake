# all: linux windows    -- `make all` builds both targets

CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O3
RAYLIBFLAGS = -lraylib

TARGET = build/snake
WIN_TARGET = build/snake.exe

# Windows cross build: run `nix develop .#windows` first. That shell exports
# RAYLIB_WIN, RAYGUI_INC and MCFGTHREAD_WIN, used by the rules below.
WIN_CXX = x86_64-w64-mingw32-g++
WIN_CPPFLAGS = -isystem $(RAYLIB_WIN)/include -isystem $(RAYGUI_INC)
WIN_LDFLAGS = -L$(RAYLIB_WIN)/lib -L$(MCFGTHREAD_WIN)/lib -static -mwindows
WIN_LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm

# Replaces the cryptic "Error 127" with an instruction
check-win-cxx = command -v $(WIN_CXX) >/dev/null 2>&1 || \
  { echo "error: $(WIN_CXX) not found. Run 'nix develop .\#windows' first."; exit 1; }

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, build/%.o, $(SRCS))
WIN_OBJS = $(patsubst %.cpp, build/%.win.o, $(SRCS))

.PHONY: all linux windows run clean

# The first rule is the default goal: Linux only, so a plain `make` keeps
# working in the default shell (no cross compiler there).
linux: $(TARGET)
windows: $(WIN_TARGET)
all: linux windows

$(TARGET): $(OBJS) | build
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(RAYLIBFLAGS)

$(WIN_TARGET): $(WIN_OBJS) | build
	@$(check-win-cxx)
	$(WIN_CXX) $(CXXFLAGS) $(WIN_CPPFLAGS) -o $@ $(WIN_OBJS) $(WIN_LDFLAGS) $(WIN_LDLIBS)

build/%.o: %.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.win.o: %.cpp | build
	@$(check-win-cxx)
	$(WIN_CXX) $(CXXFLAGS) $(WIN_CPPFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build

run: $(TARGET)
	./$(TARGET)
