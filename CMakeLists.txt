cmake_minimum_required(VERSION 3.18)
project(laval-nozzle-1D)

set(CMAKE_CXX_STANDARD 14)

add_executable(main.out
	main.c)
target_link_libraries(main.out m)
