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

EXEC_GRAPH                 = $(BIN_DIR)/graph-main
EXEC_BENCHMARK             = $(BIN_DIR)/benchmark-vaccination
EXEC_BENCHMARK_STEPSIZE    = $(BIN_DIR)/benchmark-stepsize
EXEC_BENCHMARK_LP          = $(BIN_DIR)/benchmark-lp
EXEC_DYNAMIC_LPTKR         = $(BIN_DIR)/dynamic-lptkr
EXEC_DYNAMIC_LPIRP         = $(BIN_DIR)/dynamic-lpirp
EXEC_DYNAMIC_GREEDY        = $(BIN_DIR)/dynamic-greedy
EXEC_DYNAMIC_LOCALSEARCH	= $(BIN_DIR)/dynamic-localSearch
EXEC_DYNAMIC_HILLCLIMBING  = $(BIN_DIR)/dynamic-hillclimbing

COMMON_SOURCES := $(shell find $(SRC_DIR) \
    -path $(SRC_DIR)/main -prune -o \
	-path $(SRC_DIR)/algorithms -prune -o \
    -name '*.cpp' -print)

COMMON_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, \
                              $(BUILD_DIR)/%.o, \
                              $(COMMON_SOURCES))


MAIN_GRAPH_OBJ              = $(BUILD_DIR)/main/graph-main.o
MAIN_BENCHMARK_OBJ          = $(BUILD_DIR)/main/benchmark-vaccination.o
MAIN_BENCHMARK_STEPSIZE_OBJ = $(BUILD_DIR)/main/benchmark-stepsize.o
MAIN_BENCHMARK_LP_OBJ       = $(BUILD_DIR)/main/benchmark-lp.o
MAIN_DYNAMIC_LPTKR_OBJ      = $(BUILD_DIR)/main/main-dynamic-lptkr.o
MAIN_DYNAMIC_LPIRP_OBJ      = $(BUILD_DIR)/main/main-dynamic-lpirp.o
MAIN_DYNAMIC_GREEDY_OBJ     = $(BUILD_DIR)/main/main-dynamic-greedy.o
MAIN_DYNAMIC_HILLCLIMBING_OBJ = $(BUILD_DIR)/main/main-dynamic-hillClimbing.o
MAIN_DYNAMIC_LOCALSEARCH_OBJ = $(BUILD_DIR)/main/main-dynamic-localSearch.o



all: dynamic-lptkr dynamic-lpirp dynamic-greedy dynamic-hillclimbing dynamic-localSearch

graph: $(COMMON_OBJECTS) $(MAIN_GRAPH_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_GRAPH) $^ $(LDFLAGS)

benchmark: $(COMMON_OBJECTS) $(MAIN_BENCHMARK_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_BENCHMARK) $^ \
		$(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread

benchmark-stepsize: $(COMMON_OBJECTS) $(MAIN_BENCHMARK_STEPSIZE_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_BENCHMARK_STEPSIZE) $^ \
		$(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread

benchmark-lp: $(COMMON_OBJECTS) $(MAIN_BENCHMARK_LP_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_BENCHMARK_LP) $^ \
		$(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread

dynamic-lptkr: $(COMMON_OBJECTS) $(MAIN_DYNAMIC_LPTKR_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_DYNAMIC_LPTKR) $^ \
		$(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread
	
dynamic-lpirp: $(COMMON_OBJECTS) $(MAIN_DYNAMIC_LPIRP_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_DYNAMIC_LPIRP) $^ \
		$(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread

dynamic-greedy: $(COMMON_OBJECTS) $(MAIN_DYNAMIC_GREEDY_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_DYNAMIC_GREEDY) $^ \
		$(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread

dynamic-hillclimbing: $(COMMON_OBJECTS) $(MAIN_DYNAMIC_HILLCLIMBING_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_DYNAMIC_HILLCLIMBING) $^ \
		$(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread	
	
dynamic-localSearch: $(COMMON_OBJECTS) $(MAIN_DYNAMIC_LOCALSEARCH_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_DYNAMIC_LOCALSEARCH) $^ \
		$(LDFLAGS) -I$(INCLUDES_GUROBI) $(CPPLIB) -lm -ldl -lpthread	

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(INCLUDES_GUROBI) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean graph benchmark benchmark-stepsize benchmark-lp dynamic-lptkr dynamic-lpirp dynamic-greedy dynamic-hillclimbing dynamic-localSearch