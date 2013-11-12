cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(immutable)

## Google test framework setup

# Enable external projects
include(ExternalProject)

set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/vendor)

file(WRITE ${CMAKE_BINARY_DIR}/gtest.patch "")

# Add gtest
ExternalProject_Add(
    googletest
    SVN_REPOSITORY http://googletest.googlecode.com/svn/trunk/
    SVN_REVISION -r 660
    TIMEOUT 10
    PATCH_COMMAND svn patch ${CMAKE_BINARY_DIR}/gtest.patch ${CMAKE_BINARY_DIR}/vendor/src/googletest
    # Force separate output paths for debug and release builds to allow easy
    # identification of correct lib in subsequent TARGET_LINK_LIBRARIES commands
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
               -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
               -Dgtest_force_shared_crt=ON
    # Disable install step
    INSTALL_COMMAND ""
    # Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)


# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
include_directories(${source_dir}/include)
include_directories(${source_dir}/gtest/include)

## Testing setup

enable_testing()

# Add test file

file(GLOB test_cpp "test/*_test.cpp")
add_executable(unitTests
  ${PROJECT_SOURCE_DIR}/test/main.cpp
  ${test_cpp}
)
add_dependencies(unitTests googletest)

# Linking for tests
ExternalProject_Get_Property(googletest binary_dir)

set(Suffix ".a")
set(Pthread "-pthread")
target_link_libraries(
    unitTests
    ${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${Suffix}
    ${Pthread}
)

# test
add_test(unitTests unitTests)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS unitTests)
