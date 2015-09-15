cmake_minimum_required(VERSION 3.0)

find_package(SDL2 REQUIRED)

add_library(Graphics STATIC
	GraphicalMain.cpp
	Graphics.cpp
	Graphics.h
)

target_include_directories(Graphics
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${SDL2_INCLUDE_DIR}
)

target_link_libraries(Graphics
	PUBLIC
		${SDL2_LIBRARY}
		HalideLib
)

add_library(Common STATIC
	ImageConverter.cpp
	ImageConverter.h
)

target_include_directories(Common
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Common
	PUBLIC
		HalideLib
)
