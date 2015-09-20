cmake_minimum_required(VERSION 3.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

project(proj_HalideExamples)

# Force debug symbols always
set(PROFILING_CXX_FLAGS "-g")
#set(PROFILING_LINK_FLAGS "-pg")

# Halide code needs C++11
add_definitions(-std=c++11 ${PROFILING_CXX_FLAGS})

# Define Halide library and include directories
if(NOT DEFINED HALIDE_ROOT)
    message(FATAL_ERROR "You must define HALIDE_ROOT to point to your installation of Halide.")
endif()

set(HALIDE_INCLUDE_DIR ${HALIDE_ROOT}/include)
set(HALIDE_LIBRARY_DIR ${HALIDE_ROOT}/bin)
set(HALIDE_LIBRARY_NAME Halide)

set(CMAKE_INSTALL_RPATH ${HALIDE_LIBRARY_DIR})
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# Create an interface library for Halide
add_library(HalideLib INTERFACE)
target_include_directories(HalideLib INTERFACE ${HALIDE_INCLUDE_DIR})
target_link_libraries(HalideLib INTERFACE -L${HALIDE_LIBRARY_DIR} ${HALIDE_LIBRARY_NAME})

add_subdirectory(Common)
add_subdirectory(Wave)
add_subdirectory(ParticleFountain)
add_subdirectory(Grav)
add_subdirectory(Test)
