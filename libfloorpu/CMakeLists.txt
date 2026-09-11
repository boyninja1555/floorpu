cmake_minimum_required(VERSION 4.2)
project(libfloorpu LANGUAGES C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Library
add_library(libfloorpu STATIC)
target_sources(libfloorpu PUBLIC src/ram.c src/cpu.c)
target_include_directories(libfloorpu PUBLIC include)
set_target_properties(libfloorpu PROPERTIES OUTPUT_NAME "floorpu")