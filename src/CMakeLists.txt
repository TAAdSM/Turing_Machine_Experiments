cmake_minimum_required(VERSION 3.13)
project(TuringMachineExperiments)
set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
set(CMAKE_BUILD_TYPE Debug)

add_executable(turingTest ${SOURCE_FILES})