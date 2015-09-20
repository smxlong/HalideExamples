cmake_minimum_required(VERSION 3.0)

add_executable(Grav
	Grav.cpp
)

target_link_libraries(Grav
	PUBLIC
	Graphics
	Common
)
