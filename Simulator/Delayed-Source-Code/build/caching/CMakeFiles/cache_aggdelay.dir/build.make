# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

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
CMAKE_COMMAND = /snap/cmake/1366/bin/cmake

# The command to remove a file.
RM = /snap/cmake/1366/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/fbs/la-cache-main/Simulator/Delayed-Source-Code

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build

# Include any dependencies generated for this target.
include caching/CMakeFiles/cache_aggdelay.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include caching/CMakeFiles/cache_aggdelay.dir/compiler_depend.make

# Include the progress variables for this target.
include caching/CMakeFiles/cache_aggdelay.dir/progress.make

# Include the compile flags for this target's objects.
include caching/CMakeFiles/cache_aggdelay.dir/flags.make

caching/CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.o: caching/CMakeFiles/cache_aggdelay.dir/flags.make
caching/CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.o: /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/caching/src/cache_aggdelay.cpp
caching/CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.o: caching/CMakeFiles/cache_aggdelay.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object caching/CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.o"
	cd /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build/caching && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT caching/CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.o -MF CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.o.d -o CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.o -c /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/caching/src/cache_aggdelay.cpp

caching/CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.i"
	cd /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build/caching && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/caching/src/cache_aggdelay.cpp > CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.i

caching/CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.s"
	cd /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build/caching && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/caching/src/cache_aggdelay.cpp -o CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.s

# Object files for target cache_aggdelay
cache_aggdelay_OBJECTS = \
"CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.o"

# External object files for target cache_aggdelay
cache_aggdelay_EXTERNAL_OBJECTS =

bin/cache_aggdelay: caching/CMakeFiles/cache_aggdelay.dir/src/cache_aggdelay.cpp.o
bin/cache_aggdelay: caching/CMakeFiles/cache_aggdelay.dir/build.make
bin/cache_aggdelay: lib/libhashing.a
bin/cache_aggdelay: /usr/local/lib/libboost_program_options.so.1.84.0
bin/cache_aggdelay: caching/CMakeFiles/cache_aggdelay.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/cache_aggdelay"
	cd /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build/caching && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cache_aggdelay.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
caching/CMakeFiles/cache_aggdelay.dir/build: bin/cache_aggdelay
.PHONY : caching/CMakeFiles/cache_aggdelay.dir/build

caching/CMakeFiles/cache_aggdelay.dir/clean:
	cd /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build/caching && $(CMAKE_COMMAND) -P CMakeFiles/cache_aggdelay.dir/cmake_clean.cmake
.PHONY : caching/CMakeFiles/cache_aggdelay.dir/clean

caching/CMakeFiles/cache_aggdelay.dir/depend:
	cd /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/fbs/la-cache-main/Simulator/Delayed-Source-Code /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/caching /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build/caching /home/fbs/la-cache-main/Simulator/Delayed-Source-Code/build/caching/CMakeFiles/cache_aggdelay.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : caching/CMakeFiles/cache_aggdelay.dir/depend
