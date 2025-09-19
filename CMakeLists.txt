cmake_minimum_required(VERSION 3.16)

project(GeneralVoronoiCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
# Set output directory per-target to avoid accidental absolute paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIRECTORY}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIRECTORY}/lib)

add_executable(voronoi_demo
    src/main.cpp
)

target_compile_features(voronoi_demo PRIVATE cxx_std_17)

set_target_properties(voronoi_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


