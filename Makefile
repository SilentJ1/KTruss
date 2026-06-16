CXX ?= g++
CXXFLAGS ?= -std=c++17 -O3 -Wall -Wextra -pedantic
CPPFLAGS ?= -Iinclude

BUILD_DIR := build
BIN_DIR := output
TARGET := $(BIN_DIR)/ktruss
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

.PHONY: all clean clean_all run

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp include/TrussDecomposition.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

run: $(TARGET)
	$(TARGET) $(ARGS)

clean:
	rm -rf $(BUILD_DIR)

clean_all: clean
	rm -rf $(BIN_DIR)
