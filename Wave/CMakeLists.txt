cmake_minimum_required(VERSION 3.0)

add_executable(Wave
	Wave.cpp
)

target_link_libraries(Wave
	PUBLIC
		Graphics
		${PROFILING_LINK_FLAGS}
)
