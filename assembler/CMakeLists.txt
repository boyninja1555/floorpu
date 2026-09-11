cmake_minimum_required(VERSION 4.2)
project(fpuasm LANGUAGES C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Libraries
add_subdirectory(../libfloorpu ../build/libfloorpu)
add_subdirectory(vendor/vecfloor)

# Executable
add_executable(FpuASM)
target_sources(FpuASM PRIVATE src/main.c src/lexer.c src/parser.c)
target_include_directories(FpuASM PRIVATE include)
target_link_libraries(FpuASM PRIVATE libfloorpu)
target_link_libraries(FpuASM PRIVATE vecfloor)