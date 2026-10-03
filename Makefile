# all: $(TARGET) $(WIN_TARGET)    // use this rule if you want both linux and windows executable to build

CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O3
RAYLIBFLAGS = -lraylib

TARGET = build/snake
# WIN_TARGET = build/snake.exe
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, build/%.o, $(SRCS))
# WIN_OBJS = $(patsubst %.cpp, build/%.win.o, $(SRCS))

$(TARGET): $(OBJS) | build
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(RAYLIBFLAGS)

# $(WIN_TARGET): $(WIN_OBJS) | build
	# x86_64-w64-mingw32-g++ $(CXXFLAGS) -o $(WIN_TARGET) $(WIN_OBJS)

build/%.o: %.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# build/%.win.o: %.cpp | build
	# x86_64-w64-mingw32-g++ $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build


#   make clean     // is the comand to nuke all build files to rebuild from scratch if errors appear



