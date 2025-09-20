cmake_minimum_required(VERSION 3.16)

project(GeneralVoronoiCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
# Set output directory per-target to avoid accidental absolute paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIRECTORY}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIRECTORY}/lib)

option(BUILD_TOOLS "Build CLI tools" ON)
option(WITH_OPENCV "Enable OpenCV for image I/O and EDT" ON)
option(WITH_OPENMP "Enable OpenMP parallelization where applicable" ON)
option(USE_NATIVE_OPTIMIZATIONS "Build with -march=native when using GCC/Clang" ON)

if(WITH_OPENCV)
    find_package(OpenCV QUIET COMPONENTS core imgproc imgcodecs)
    if(OpenCV_FOUND)
        message(STATUS "OpenCV found: ${OpenCV_VERSION}")
        add_compile_definitions(GVD_TOPO_WITH_OPENCV)
    else()
        message(STATUS "OpenCV not found; building without it")
    endif()
endif()

if(WITH_OPENMP)
    find_package(OpenMP QUIET)
endif()

if(USE_NATIVE_OPTIMIZATIONS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-march=native)
    endif()
endif()

add_library(gvd_topo
    src/main.cpp
    src/OccupancyGrid.cpp
    src/GvdGenerator.cpp
    src/TopologyExtractor.cpp
    src/ros_adapters.cpp
    src/parameters.cpp
)
target_compile_features(gvd_topo PRIVATE cxx_std_17)
if(OpenMP_CXX_FOUND)
    target_link_libraries(gvd_topo PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(gvd_topo PUBLIC GVD_TOPO_WITH_OPENMP)
endif()

target_include_directories(gvd_topo PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(gvd_topo PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
if(OpenCV_FOUND)
    target_link_libraries(gvd_topo PUBLIC ${OpenCV_LIBS})
endif()

if(BUILD_TOOLS)
    add_executable(gvd_topo_cli
        src/cli_main.cpp
    )
    target_link_libraries(gvd_topo_cli PRIVATE gvd_topo)
    set_target_properties(gvd_topo_cli PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    add_executable(test_ros_adapters
        src/test_ros_adapters.cpp
    )
    target_link_libraries(test_ros_adapters PRIVATE gvd_topo)
    set_target_properties(test_ros_adapters PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    add_executable(create_test_maps
        src/create_test_maps.cpp
    )
    target_link_libraries(create_test_maps PRIVATE gvd_topo)
    set_target_properties(create_test_maps PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    add_executable(regression_test
        src/regression_test.cpp
    )
    target_link_libraries(regression_test PRIVATE gvd_topo)
    set_target_properties(regression_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()


