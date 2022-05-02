cmake_minimum_required(VERSION 3.10)
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)

project(py_cpp_sample CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

# Use Python
find_package(Python 3.8 REQUIRED)
find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)
find_package(PythonLibs 3.8 REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# Use Boost C++ libraries
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_NO_SYSTEM_PATHS TRUE)
set(BOOST_ROOT "/opt/boost")
find_package(Boost 1.78 REQUIRED COMPONENTS python numpy)

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

# Executable unit tests
pybind11_add_module(py_cpp_sample_cpp_impl ../src/cpp_impl/popcount.cpp ../src/cpp_impl/popcount_impl.cpp)
add_executable(test_popcount ../src/cpp_impl/popcount.cpp ../src/cpp_impl/popcount_impl.cpp ../src/cpp_impl_boost/popcount_boost.cpp ../src/cpp_impl_boost/popcount_impl_boost.cpp test_popcount.cpp)
target_compile_options(test_popcount PRIVATE -Wall -Wextra -Wconversion -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wno-unused-parameter)
target_include_directories(test_popcount SYSTEM PRIVATE ${PYTHON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_include_directories(test_popcount PRIVATE "${BASEPATH}" "${BASEPATH}/../src/cpp_impl" "${BASEPATH}/../src/cpp_impl_boost")
target_link_libraries(test_popcount "${Boost_LIBRARIES}" "${PYTHON_LIBRARIES}" gtest_main pthread)
gtest_add_tests(TARGET test_popcount)
