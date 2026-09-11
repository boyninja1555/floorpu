cmake_minimum_required(VERSION 4.2)
project(floorpu LANGUAGES C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Libraries
add_subdirectory(libfloorpu)

# Executable
add_executable(FloorPU)
target_sources(FloorPU PRIVATE src/main.c)
target_include_directories(FloorPU PRIVATE include)
target_link_libraries(FloorPU PRIVATE libfloorpu)