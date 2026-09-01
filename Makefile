CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -O3 -march=native -mtune=native -Iinclude

TARGET := generator

SRC := \
	src/main.cpp \
	src/TableGenerator.cpp

OBJ := $(SRC:src/%.cpp=build/%.o)
DEP := $(OBJ:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEP)

clean:
	rm -rf build $(TARGET)

.PHONY: all clean