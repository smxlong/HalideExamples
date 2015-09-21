cmake_minimum_required(VERSION 3.0)

add_executable(SpringMesh
	SpringMesh.cpp
)

target_link_libraries(SpringMesh
	PUBLIC
	Graphics
	Common
)
