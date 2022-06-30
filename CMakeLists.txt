cmake_minimum_required(VERSION 3.20)
project(ProgrammingLanguages)

set(CMAKE_CXX_STANDARD 14)

add_executable(ProgrammingLanguages lodepng.h lodepng.cpp parallelization/ParallelComputationsYellow.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")