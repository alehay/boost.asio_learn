# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kudakov/temp_poj/boos_lern

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kudakov/temp_poj/boos_lern/build

# Include any dependencies generated for this target.
include async/CMakeFiles/asinc_read.dir/depend.make

# Include the progress variables for this target.
include async/CMakeFiles/asinc_read.dir/progress.make

# Include the compile flags for this target's objects.
include async/CMakeFiles/asinc_read.dir/flags.make

async/CMakeFiles/asinc_read.dir/asinc_client_read.cpp.o: async/CMakeFiles/asinc_read.dir/flags.make
async/CMakeFiles/asinc_read.dir/asinc_client_read.cpp.o: ../async/asinc_client_read.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kudakov/temp_poj/boos_lern/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object async/CMakeFiles/asinc_read.dir/asinc_client_read.cpp.o"
	cd /home/kudakov/temp_poj/boos_lern/build/async && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/asinc_read.dir/asinc_client_read.cpp.o -c /home/kudakov/temp_poj/boos_lern/async/asinc_client_read.cpp

async/CMakeFiles/asinc_read.dir/asinc_client_read.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/asinc_read.dir/asinc_client_read.cpp.i"
	cd /home/kudakov/temp_poj/boos_lern/build/async && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kudakov/temp_poj/boos_lern/async/asinc_client_read.cpp > CMakeFiles/asinc_read.dir/asinc_client_read.cpp.i

async/CMakeFiles/asinc_read.dir/asinc_client_read.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/asinc_read.dir/asinc_client_read.cpp.s"
	cd /home/kudakov/temp_poj/boos_lern/build/async && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kudakov/temp_poj/boos_lern/async/asinc_client_read.cpp -o CMakeFiles/asinc_read.dir/asinc_client_read.cpp.s

# Object files for target asinc_read
asinc_read_OBJECTS = \
"CMakeFiles/asinc_read.dir/asinc_client_read.cpp.o"

# External object files for target asinc_read
asinc_read_EXTERNAL_OBJECTS =

async/asinc_read: async/CMakeFiles/asinc_read.dir/asinc_client_read.cpp.o
async/asinc_read: async/CMakeFiles/asinc_read.dir/build.make
async/asinc_read: ../libs/boost/liblib-asio.a
async/asinc_read: ../libs/boost/liblib-beast.a
async/asinc_read: ../libs/boost/libboost_json.a
async/asinc_read: async/CMakeFiles/asinc_read.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kudakov/temp_poj/boos_lern/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable asinc_read"
	cd /home/kudakov/temp_poj/boos_lern/build/async && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/asinc_read.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
async/CMakeFiles/asinc_read.dir/build: async/asinc_read

.PHONY : async/CMakeFiles/asinc_read.dir/build

async/CMakeFiles/asinc_read.dir/clean:
	cd /home/kudakov/temp_poj/boos_lern/build/async && $(CMAKE_COMMAND) -P CMakeFiles/asinc_read.dir/cmake_clean.cmake
.PHONY : async/CMakeFiles/asinc_read.dir/clean

async/CMakeFiles/asinc_read.dir/depend:
	cd /home/kudakov/temp_poj/boos_lern/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kudakov/temp_poj/boos_lern /home/kudakov/temp_poj/boos_lern/async /home/kudakov/temp_poj/boos_lern/build /home/kudakov/temp_poj/boos_lern/build/async /home/kudakov/temp_poj/boos_lern/build/async/CMakeFiles/asinc_read.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : async/CMakeFiles/asinc_read.dir/depend

