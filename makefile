# PROJECT STRUCTURE:
#      Telomeri
#       |- src       # Source files. Can contain subdirectories.
#       |- include   # Header files. Can contain subdirectories.
#       |- lib       # Project specific libraries.
#       |- obj       # Intermediate (object) files. Mirrors source tree.
#       |- res       # Project resources.
#       |- exe       # Executable. Built with 'make'.
#       |- test      # Test source files.
#           |- obj   # Object files of test sources. Mirrors test source tree.
#           |- exe   # Test executable. Built with 'make test'.


# ------------------------- GENERAL PROJECT SETTINGS ---------------------------
EXECUTABLE = Telomeri
CC = gcc
GXX = g++

# Directory structure.
INCLUDE_DIR = include
SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = lib
TEST_DIR = test

# File containing main function definition. Excluded from test compilation.
MAIN_SOURCE_FILE = src/Main.cpp
# ------------------------------------------------------------------------------


# ------------------------------- COMPILER FLAGS -------------------------------
# Debugging flags. Remove if not debugging to enable optimisations. 
DEBUG_FLAGS = -ggdb -O0 -DDEBUG

# Compiler flags (common, C++ only and C only).
CPPFLAGS = -Wall -pedantic-errors -O2 -I$(INCLUDE_DIR) $(DEBUG_FLAGS)
CXXFLAGS = -xc++ -std=c++17 
CFLAGS = -xc -std=c17
# ------------------------------------------------------------------------------


# -------------------------------- LINKER FLAGS --------------------------------
# Linker flags and libraries to link against.
LDFLAGS = 
LDLIBS = 

# Link against project specific libraries in 'lib' directory.
LDFLAGS += -Llib

# Link against C++ standard libraries.
LDFLAGS += -lstdc++ -shared-libgcc 

# Link against math library. Remove if not needed.
LDFLAGS += -lm
# ------------------------------------------------------------------------------

 
# ------------------------------ HELPER COMMANDS -------------------------------
# Directory guard. Used to create directory if a file requires it.
DIRECTORY_GUARD = @mkdir -p $(@D)
# ------------------------------------------------------------------------------


# --------------------------- SOURCE AND OBJECT FILES --------------------------
# All source files.
SRC = $(shell find $(SRC_DIR) -name '*.c' -o -name '*.cpp')

# All object files.
OBJ = $(SRC:$(SRC_DIR)/%=$(OBJ_DIR)/%.o)
# ------------------------------------------------------------------------------


# --------------------------------- PHONY RULES --------------------------------
# Special words. Ignore files with those names.
.PHONY: all clean test purge
# ------------------------------------------------------------------------------


# ------------------------------- DEFAULT TARGET -------------------------------
# Build the executable. 
all: $(EXECUTABLE)

# Linking.
$(EXECUTABLE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ 

# Compiling *.c files.
$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c
	$(DIRECTORY_GUARD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Compiling *.cpp files.
$(OBJ_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	$(DIRECTORY_GUARD)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
# ------------------------------------------------------------------------------


# ---------------------------------- TESTING -----------------------------------
# Test sources (only test files).
TEST_SRC = $(shell find $(TEST_DIR) -name '*.c' -o -name '*.cpp')

# Test object files (only test files).
TEST_OBJ = $(TEST_SRC:$(TEST_DIR)/%=$(TEST_DIR)/$(OBJ_DIR)/%.o)

# Object file containing 'main()' function.
MAIN_OBJ = $(MAIN_SOURCE_FILE:$(SRC_DIR)/%=$(OBJ_DIR)/%.o)

# Build the test executable. 
test: $(TEST_DIR)/$(EXECUTABLE)

# Linking the test executable. Filter out object file that contains 'main()' 
# definition containing main since testing framework provides entry point.
$(TEST_DIR)/$(EXECUTABLE): $(filter-out $(MAIN_OBJ), $(OBJ)) $(TEST_OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ 
       
# Compile test *.cpp sources.
$(TEST_DIR)/$(OBJ_DIR)/%.cpp.o: $(TEST_DIR)/%.cpp
	$(DIRECTORY_GUARD)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Compile test *.c sources.
$(TEST_DIR)/$(OBJ_DIR)/%.c.o: $(TEST_DIR)/%.c
	$(DIRECTORY_GUARD)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
# ------------------------------------------------------------------------------


# --------------------------------- CLEANING -----------------------------------
# Remove object files.
clean:
	$(RM) $(OBJ) $(TEST_OBJ)

# Remove object directories and executable files.
purge:
	$(RM) $(OBJ_DIR) $(TEST_DIR)/$(OBJ_DIR) -r
	$(RM) $(TEST_DIR)/$(EXECUTABLE) $(EXECUTABLE)
# ------------------------------------------------------------------------------
