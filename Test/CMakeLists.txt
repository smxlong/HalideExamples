cmake_minimum_required(VERSION 3.0)

add_executable(TestImageConverter
	TestImageConverter.cpp
)

target_link_libraries(TestImageConverter
	PUBLIC
		Common
)
