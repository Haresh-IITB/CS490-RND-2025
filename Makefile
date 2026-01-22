CXX = g++
CXXFLAGS = -Wall -g -std=c++17 -O3 -march=native -mtune=native -ffast-math -fno-math-errno -funsafe-math-optimizations -flto -funroll-loops
INCLUDES = -Iinclude

INCLUDES_GUROBI=/Library/gurobi1300/macos_universal2/include
CPPLIB=-L$(INCLUDES_GUROBI)/../lib -lgurobi_c++ -lgurobi130

SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

HOMEBREW_PREFIX := /opt/homebrew

CXXFLAGS += -I$(HOMEBREW_PREFIX)/include
LDFLAGS  += -L$(HOMEBREW_PREFIX)/lib -lglpk -Wl,-rpath,$(HOMEBREW_PREFIX)/lib

EXEC_SIM = $(BIN_DIR)/simulation
EXEC_GRAPH = $(BIN_DIR)/graph-main
EXEC_BENCHMARK_VACCINATION = $(BIN_DIR)/benchmark-vaccination
EXEC_BENCHMARK_STEPSIZE = $(BIN_DIR)/benchmark-stepsize
EXEC_BENCHMARK_LP = $(BIN_DIR)/benchmark-lp

COMMON_SOURCES = $(shell find $(SRC_DIR) -name '*.cpp' ! -name 'main.cpp' ! -name 'graph-main.cpp' ! -name 'benchmark-vaccination.cpp' ! -name 'benchmark-stepsize.cpp' ! -name 'benchmark-lp.cpp' ! -name 'main-dynamic-lp.cpp' ! -name 'main_dynamic.cpp')
COMMON_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(COMMON_SOURCES))
MAIN_SIM_OBJ = $(BUILD_DIR)/main.o
MAIN_GRAPH_OBJ = $(BUILD_DIR)/graph-main.o
MAIN_DYNAMIC_LP_OBJ = $(BUILD_DIR)/main-dynamic-lp.o
MAIN_DYNAMIC_OBJ = $(BUILD_DIR)/main-dynamic.o


all: $(EXEC_SIM) $(EXEC_GRAPH) $(EXEC_BENCHMARK_VACCINATION)

dynamic-lp : $(COMMON_OBJECTS) $(BUILD_DIR)/main-dynamic-lp.o
	@echo "Linking dynamic-lp executable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/dynamic-lp $^ $(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread
	@echo "Dynamic LP build finished at $(BIN_DIR)/dynamic-lp"

benchmark-lp : $(COMMON_OBJECTS) $(BUILD_DIR)/benchmark-lp.o
	@echo "Linking benchmark-lp executable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_BENCHMARK_LP) $^ $(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread
	@echo "Benchmark LP build finished at $(EXEC_BENCHMARK_LP)"

benchmark : $(COMMON_OBJECTS) $(BUILD_DIR)/benchmark-vaccination.o
	@echo "Linking benchmark-vaccination executable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_BENCHMARK_VACCINATION) $^ $(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread
	@echo "Benchmark vaccination build finished at $(EXEC_BENCHMARK_VACCINATION)"

benchmark-stepsize : $(COMMON_OBJECTS) $(BUILD_DIR)/benchmark-stepsize.o
	@echo "Linking benchmark-stepsize executable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_BENCHMARK_STEPSIZE) $^ $(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread
	@echo "Benchmark stepsize build finished at $(EXEC_BENCHMARK_STEPSIZE)"


$(EXEC_SIM): $(COMMON_OBJECTS) $(MAIN_SIM_OBJ)
	@echo "Linking simulation executable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Simulation build finished at $(EXEC_SIM)"

$(EXEC_GRAPH): $(COMMON_OBJECTS) $(MAIN_GRAPH_OBJ)
	@echo "Linking graph-main executable..."
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Graph build finished at $(EXEC_GRAPH)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $^ -I$(INCLUDES_GUROBI) 

test_greedy_ic:
	g++ -std=c++17 -O0 -g \
		src/test_greedy_ic.cpp \
		src/graphs/waxman-graph.cpp \
		src/models/Independent-cascade.cpp \
		src/algorithms/greedy.cpp \
		src/utils/Random_number_generator.cpp \
		src/utils/SpatialGrid.cpp \
		-Iinclude \
		-o bin/test_greedy_ic

test_greedy_lt:
	g++ -std=c++17 -O0 -g \
		src/test_greedy_ic.cpp \
		src/graphs/waxman-graph.cpp \
		src/models/Linear-threshold.cpp \
		src/algorithms/greedy.cpp \
		src/utils/Random_number_generator.cpp \
		src/utils/SpatialGrid.cpp \
		-Iinclude \
		-o bin/test_greedy_ic

test_pg_lt:
	g++ -std=c++17 -O0 -g \
		src/test_pg_ic.cpp \
		src/graphs/waxman-graph.cpp \
		src/models/Linear-threshold.cpp \
		src/algorithms/page-rank.cpp \
		src/utils/Random_number_generator.cpp \
		src/utils/SpatialGrid.cpp \
		-Iinclude \
		-o bin/test_pg_lt
clean:
	@echo "Cleaning build files..."
	rm -rf $(BIN_DIR) $(BUILD_DIR)
	@echo "Clean complete."

.PHONY: all clean
