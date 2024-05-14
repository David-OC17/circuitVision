# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -I/usr/include/opencv4
CXXFLAGS_RASP = -I/usr/local/include/opencv4
INCLUDE_DIR = ./include
CXXFLAGS += -I$(INCLUDE_DIR)

# Linker flags
LDFLAGS = -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lwiringPi 
LDFLAGS_NOUART = -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lwiringPi -lopencv_videoio

# Source file
SRCS = src/main.cpp src/PCB_inspection.cpp src/aux_functions.cpp src/coms.cpp 
SRCS_NOUART = src/main.cpp src/PCB_inspection.cpp src/aux_functions.cpp src/coms.cpp test/src/coms_test.cpp 
OBJS = $(SRCS:.cpp=.o)

# Executable output
EXEC = ./bin/main
RASP = ./bin/main
DEBUG = ./bin/main_debug
RASP_DEBUG = ./bin/rasp_debug
TEST_NOUART = ./bin/main


# Default target
all: $(EXEC)

# Compile and link
$(EXEC): $(SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

rasp: $(SRCS_NOUART)
	$(CXX) $(CXXFLAGS_RASP) $^ -o $(RASP) $(LDFLAGS)

# Build individual tests
noUART: $(SRCS_NOUART)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_NOUART) $(LDFLAGS_NOUART)

# Build with debug option
debug: $(SRCS)
	$(CXX) $(CXXFLAGS) -g $^ -o $(DEBUG) $(LDFLAGS)

raspDebug: $(SRCS_NOUART)
	$(CXX) $(CXXFLAGS_RASP) -g $^ -o $(RASP_DEBUG) $(LDFLAGS)

# Clean target
clean:
	rm -f $(EXEC) $(TEST1) $(DEBUG) $(RASP_DEBUG)
