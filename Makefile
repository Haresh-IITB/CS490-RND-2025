CXX = g++
CXXFLAGS = -Wall -g
INCLUDES = -Iinclude

SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

EXECUTABLE = $(BIN_DIR)/simulation

SOURCES = $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo "Linking executable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build finished. Executable is at $(EXECUTABLE)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $^

clean:
	@echo "Cleaning up build files..."
	rm -rf $(BIN_DIR) $(BUILD_DIR)
	@echo "Clean complete."

.PHONY: all clean

