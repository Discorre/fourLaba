# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -pthread

# Source files
SOURCES = Zad1.cpp Zad2.cpp Zad3.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable names
EXECUTABLES = zad1 zad2 zad3

# Default target
all: $(EXECUTABLES)



# Compile each source file into an object file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link object files to create the executables
zad1: Zad1.o
	$(CXX) $(CXXFLAGS) $^ -o $@
	rm -f Zad1.o

zad2: Zad2.o
	$(CXX) $(CXXFLAGS) $^ -o $@
	rm -f Zad2.o

zad3: Zad3.o
	$(CXX) $(CXXFLAGS) $^ -o $@
	rm -f Zad3.o

receipts:
	python3 1.py

# Clean up build files
clean:
	rm -f $(EXECUTABLES)
	rm -f receiptsUltraMini.txt receiptsMini.txt receiptsMacro.txt receiptsUltraMacro.txt