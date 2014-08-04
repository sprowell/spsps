# Build Elision.
#

# Verify the version of CMake required.
cmake_minimum_required( VERSION 2.8 )

# Define the project and the current version numbers.
project( spsps )
SET( SPSPS_MAJOR 0 )
SET( SPSPS_MINOR 0 )
SET( SPSPS_VERSION ${SPSPS_MAJOR}.${SPSPS_MINOR} )
SET( SPSPS_DIST_NAME "spsps-${SPSPS_VERSION}" )
SET( MACOSX_RPATH 1 )
if (POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

# Locate valgrind.
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)

# Specify the list of source files to use for the core library here.
FILE(GLOB_RECURSE lib_sources RELATIVE ${CMAKE_HOME_DIRECTORY} c/*.c)

# Find include files.
include_directories( AFTER SYSTEM c )

# Figure out if this is a debug or release.
if( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE "Release" )
endif( NOT CMAKE_BUILD_TYPE )

# Modify the include path so we can find our own files.
include_directories( AFTER SYSTEM src )

# Build the core library.
add_library( spsps SHARED ${lib_sources} )

# Build the JSON example.
add_executable( json examples/json.c )
target_link_libraries( json spsps )

# Build the tests.
add_executable( string_test test/string_test.c )
target_link_libraries( string_test spsps )

