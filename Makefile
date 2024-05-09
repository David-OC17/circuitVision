# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -I/usr/include/opencv4
INCLUDE_DIR = ./include
CXXFLAGS += -I$(INCLUDE_DIR)

# Linker flags
LDFLAGS = -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui 

# Source file
SRCS = src/main.cpp src/PCB_inspection.cpp src/aux_functions.cpp
SRCS_TEST1= test/contour_finder_test.cpp src/PCB_inspection.cpp
OBJS = $(SRCS:.cpp=.o)

# Executable output
EXEC = ./bin/main

DEBUG = ./bin/main_debug
TEST1 = ./bin/contour_finder_test


# Default target
all: $(EXEC)

# Compile and link
$(EXEC): $(SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)


# Build individual tests
contourFinderTest: $(SRCS_TEST1)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST1) $(LDFLAGS)

# Build with debug option
debug: $(SRCS)
	$(CXX) $(CXXFLAGS) -g $^ -o $(DEBUG) $(LDFLAGS)

# Clean target
clean:
	rm -f $(EXEC) $(TEST1) $(DEBUG)
