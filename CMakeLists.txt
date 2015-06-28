# Build SPSPS.
#

# Verify the version of CMake required.
cmake_minimum_required( VERSION 2.8 )

# Define the project and the current version numbers.  This project
# uses Semantic Versioning 2.0.0 (see http://semver.org/spec/v2.0.0.html).
project( spsps )
SET( SPSPS_MAJOR 0 )
SET( SPSPS_MINOR 0 )
SET( SPSPS_PATCH 0 )

# Create some strings.
SET( SPSPS_VERSION ${SPSPS_MAJOR}.${SPSPS_MINOR}.${SPSPS_PATCH} )
SET( SPSPS_DIST_NAME "spsps-${SPSPS_VERSION}" )

# Some CMake configuration.
# Read about RPATH here:
# http://www.cmake.org/Wiki/CMake_RPATH_handling
# Read about related policy MP0042 here:
# http://www.cmake.org/cmake/help/v3.0/policy/CMP0042.html
SET( MACOSX_RPATH 1 )
if (POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

# Locate valgrind.  We use valgrind to check for memory leaks.
find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)

# Specify the list of source files to use for the core library here.
FILE(GLOB_RECURSE lib_sources RELATIVE ${CMAKE_HOME_DIRECTORY} c/*.c)

# Find include files.
include_directories( AFTER SYSTEM include )

# Turn on all warnings and errors, and use C++11 extensions.
if( CMAKE_COMPILER_IS_GNUCC )
     SET ( CMAKE_C_FLAGS "-D_BSD_SOURCE -std=c11 -pedantic -Wall" )
endif( CMAKE_COMPILER_IS_GNUCC )

# Figure out if this is a debug or release.
if( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE "Release" )
endif( NOT CMAKE_BUILD_TYPE )

# Build the core library.
add_library( spsps_shared SHARED ${lib_sources} )
add_library( spsps_static STATIC ${lib_sources} )
set_property( TARGET spsps_static PROPERTY POSITION_INDEPENDENT_CODE 1 )
if( UNIX )
    set_target_properties(spsps_shared PROPERTIES OUTPUT_NAME spsps)
    set_target_properties(spsps_static PROPERTIES OUTPUT_NAME spsps)
endif ( UNIX )

# Build the JSON example.
add_executable( json test/json_test.c )
target_link_libraries( json spsps_shared m )

# Build the TOML example.
#add_executable( toml test/toml_test.c )
#target_link_libraries( toml spsps m )

# Build the tests.
add_executable( string_test test/string_test.c )
target_link_libraries( string_test spsps_shared m )
add_executable( double_parser test/double_parser.c )
target_link_libraries( double_parser spsps_shared m )

# Add a documentation target.  First we have to find doxygen.
find_program( doxygen_path doxygen PATHS ENV PATH NO_DEFAULT_PATH )
if( doxygen_path )
    set( doc_path "${CMAKE_CURRENT_BINARY_DIR}/doc/api" )
    message( "Found Doxygen at ${doxygen_path}." )
    message( "Writing API documentation to ${doc_path}" )
    FILE( MAKE_DIRECTORY "${doc_path}" )
    add_custom_target(
        doc
        ${doxygen_path} ${CMAKE_CURRENT_SOURCE_DIR}/spsps.doxyfile
        working_directory ${CMAKE_CURRENT_SOURCE_DIR}
    )
else( doxygen_path )
    message( "WARNING: Doxygen not found; not generating documentation." )
    add_custom_target(
        doc
        echo "WARNING: Doxygen not found; not generating documentation."
    )
endif( doxygen_path )
