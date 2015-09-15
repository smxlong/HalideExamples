cmake_minimum_required(VERSION 3.0)

project(proj_HalideExamples)

set(PROFILING_CXX_FLAGS "-g")
#set(PROFILING_LINK_FLAGS "-pg")

# Halide code needs C++11
add_definitions(-std=c++11 ${PROFILING_CXX_FLAGS})

# Define Halide library and include directories
set(HALIDE_INCLUDE_DIR /home/scott/opt/halide/include)
set(HALIDE_LIBRARY_DIR /home/scott/opt/halide/bin)
set(HALIDE_LIBRARY Halide)

# Create an interface library for Halide
add_library(HalideLib INTERFACE)
target_include_directories(HalideLib INTERFACE ${HALIDE_INCLUDE_DIR})
target_link_libraries(HalideLib INTERFACE -L${HALIDE_LIBRARY_DIR} ${HALIDE_LIBRARY})

add_subdirectory(Common)
add_subdirectory(Wave)
add_subdirectory(Test)
