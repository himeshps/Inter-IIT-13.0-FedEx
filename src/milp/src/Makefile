# Compiler and flags
CXX = g++
CXXFLAGS = -I$(GUROBI_HOME)/include -std=c++11
LDFLAGS = -L$(GUROBI_HOME)/lib -lgurobi_c++ -lgurobi120 -lm

# Source files and output
SRCS = solver.cpp pre_process.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = program

# Default rule
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
