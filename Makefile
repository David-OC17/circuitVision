# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -I/usr/include/opencv4
INCLUDE_DIR = ./include
CXXFLAGS += -I$(INCLUDE_DIR)

# Linker flags
LDFLAGS = -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui 

# Source file
SRCS = src/main.cpp src/PCB_inspection.cpp
OBJS = $(SRCS:.cpp=.o)

# Executable output
EXEC = ./bin/main

# Default target
all: $(EXEC)

# Compile and link
$(EXEC): $(SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Clean target
clean:
	rm -f $(EXEC)
