cmake_minimum_required(VERSION 3.16)
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)

project(rCppSample CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

# Based on
# https://github.com/abandonware-pjz37/rcpp-test/blob/master/cmake/FindR.cmake
set(R_ROOT_DIR "$ENV{R_HOME}")
set(RLIB "R")
find_path(R_INCLUDE_DIR R.h HINTS "${R_ROOT_DIR}" /usr/share /opt PATH_SUFFIXES include R/include)
find_library(R_LIBRARY NAMES "${RLIB}" HINTS "${R_ROOT_DIR}" PATH_SUFFIXES lib R/lib)

file(GLOB_RECURSE RCPP_INCLUDE_HEADER "${R_ROOT_DIR}/*/Rcpp.h")
if(NOT RCPP_INCLUDE_HEADER)
file(GLOB_RECURSE RCPP_INCLUDE_HEADER "/home/*/Rcpp.h")
endif()

get_filename_component(RCPP_INCLUDE_DIR "${RCPP_INCLUDE_HEADER}" DIRECTORY)
find_path(RCPP_INCLUDE_DIR "Rcpp.h" HINTS "${RCPP_INCLUDE_DIR}")
set(R_INCLUDES_DIRS "${R_INCLUDE_DIR}" "${RCPP_INCLUDE_DIR}")

# Use Google Test
# Based on
# https://mola1129.dev/cmake-googletets
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . RESULT_VARIABLE result WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)

if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build . RESULT_VARIABLE result WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)

if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src ${CMAKE_CURRENT_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)

enable_testing()
include(GoogleTest)

# Include header files
set(BASEPATH "${CMAKE_SOURCE_DIR}")
set(COMMON_INCLUDE_DIRECTORIES "${BASEPATH}" "${BASEPATH}/../src")

# Executable unit tests with Rcpp
set(COMMON_COMPILE_OPTIONS -DSTRICT_R_HEADERS -Wall -Wextra -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wno-unused-parameter)
add_executable(test_popcount ../src/popcount.cpp test_popcount.cpp)

target_compile_options(test_popcount PRIVATE ${COMMON_COMPILE_OPTIONS})
target_include_directories(test_popcount SYSTEM PRIVATE ${R_INCLUDES_DIRS})
target_include_directories(test_popcount PRIVATE ${COMMON_INCLUDE_DIRECTORIES})
target_link_libraries(test_popcount "${R_LIBRARY}" gtest_main)
#target_precompile_headers(test_popcount PRIVATE ../src/test_popcount.h)
gtest_add_tests(TARGET test_popcount)

# Executable unit tests without Rcpp
add_executable(test_popcount_std ../src/popcount.cpp test_popcount.cpp)
target_compile_options(test_popcount_std PRIVATE -DUNIT_TEST_CPP ${COMMON_COMPILE_OPTIONS})
target_include_directories(test_popcount_std SYSTEM PRIVATE ${R_INCLUDES_DIRS})
target_include_directories(test_popcount_std PRIVATE ${COMMON_INCLUDE_DIRECTORIES})
target_link_libraries(test_popcount_std "${R_LIBRARY}" gtest_main)
#target_precompile_headers(test_popcount_std PRIVATE ../src/test_popcount.h)
gtest_add_tests(TARGET test_popcount_std TEST_SUFFIX _Std)
