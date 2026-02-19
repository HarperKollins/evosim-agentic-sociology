cmake_minimum_required(VERSION 3.10)
project(EvoSim LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization for release builds (important for neural network performance)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# All source files (header-only design — only main.cpp needs compiling)
add_executable(EvoSim main.cpp)

# Include directories (for header resolution)
target_include_directories(EvoSim PRIVATE ${CMAKE_SOURCE_DIR})

# Platform-specific settings
if(WIN32)
    # Ensure console output works properly on Windows
    target_compile_definitions(EvoSim PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
