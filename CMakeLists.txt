cmake_minimum_required(VERSION 3.20)
project(bf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bf src/main.cpp)

target_compile_options(bf PRIVATE -Wall -Wextra -Wpedantic)