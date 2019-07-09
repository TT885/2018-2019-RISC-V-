cmake_minimum_required(VERSION 3.5)
project(RISCV)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX -O2)
add_executable(code main.cpp)
