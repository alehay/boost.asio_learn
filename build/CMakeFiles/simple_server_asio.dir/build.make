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
include CMakeFiles/simple_server_asio.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/simple_server_asio.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/simple_server_asio.dir/flags.make

CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.o: CMakeFiles/simple_server_asio.dir/flags.make
CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.o: ../simple_server_asio.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kudakov/temp_poj/boos_lern/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.o -c /home/kudakov/temp_poj/boos_lern/simple_server_asio.cpp

CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kudakov/temp_poj/boos_lern/simple_server_asio.cpp > CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.i

CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kudakov/temp_poj/boos_lern/simple_server_asio.cpp -o CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.s

# Object files for target simple_server_asio
simple_server_asio_OBJECTS = \
"CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.o"

# External object files for target simple_server_asio
simple_server_asio_EXTERNAL_OBJECTS =

simple_server_asio: CMakeFiles/simple_server_asio.dir/simple_server_asio.cpp.o
simple_server_asio: CMakeFiles/simple_server_asio.dir/build.make
simple_server_asio: ../libs/boost/liblib-asio.a
simple_server_asio: ../libs/boost/liblib-beast.a
simple_server_asio: ../libs/boost/libboost_json.a
simple_server_asio: CMakeFiles/simple_server_asio.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kudakov/temp_poj/boos_lern/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable simple_server_asio"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/simple_server_asio.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/simple_server_asio.dir/build: simple_server_asio

.PHONY : CMakeFiles/simple_server_asio.dir/build

CMakeFiles/simple_server_asio.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/simple_server_asio.dir/cmake_clean.cmake
.PHONY : CMakeFiles/simple_server_asio.dir/clean

CMakeFiles/simple_server_asio.dir/depend:
	cd /home/kudakov/temp_poj/boos_lern/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kudakov/temp_poj/boos_lern /home/kudakov/temp_poj/boos_lern /home/kudakov/temp_poj/boos_lern/build /home/kudakov/temp_poj/boos_lern/build /home/kudakov/temp_poj/boos_lern/build/CMakeFiles/simple_server_asio.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/simple_server_asio.dir/depend
