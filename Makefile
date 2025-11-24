CXX = g++
CXXFLAGS = -Wall -g -std=c++17
INCLUDES = -Iinclude

SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

EXEC_SIM = $(BIN_DIR)/simulation
EXEC_GRAPH = $(BIN_DIR)/graph-main

COMMON_SOURCES = $(shell find $(SRC_DIR) -name '*.cpp' ! -name 'main.cpp' ! -name 'graph-main.cpp')
COMMON_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(COMMON_SOURCES))
MAIN_SIM_OBJ = $(BUILD_DIR)/main.o
MAIN_GRAPH_OBJ = $(BUILD_DIR)/graph-main.o


all: $(EXEC_SIM) $(EXEC_GRAPH)
$(EXEC_SIM): $(COMMON_OBJECTS) $(MAIN_SIM_OBJ)
	@echo "Linking simulation executable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Simulation build finished at $(EXEC_SIM)"

$(EXEC_GRAPH): $(COMMON_OBJECTS) $(MAIN_GRAPH_OBJ)
	@echo "Linking graph-main executable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Graph build finished at $(EXEC_GRAPH)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $^

clean:
	@echo "Cleaning build files..."
	rm -rf $(BIN_DIR) $(BUILD_DIR)
	@echo "Clean complete."

.PHONY: all clean
