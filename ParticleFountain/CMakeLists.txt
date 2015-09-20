cmake_minimum_required(VERSION 3.0)

add_definitions(-ffast-math)			# For faster sin, cos

add_executable(ParticleFountain
	ParticleFountain.cpp
)

target_link_libraries(ParticleFountain
	PUBLIC
		Graphics
		${PROFILING_LINK_FLAGS}
)

install(TARGETS ParticleFountain
	RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/build-dir/bin
)