/**
 * @file gvd_topo.hpp
 * @brief Main header file for the General Voronoi Diagram to Topological Map library
 * 
 * This is the main entry point for the library. Include this file to access
 * all the core functionality of the GVD topology extraction library.
 */

#pragma once

// Core functionality
#include "gvd_topo/core/OccupancyGrid.hpp"
#include "gvd_topo/core/GvdGenerator.hpp"
#include "gvd_topo/core/TopologyExtractor.hpp"
#include "gvd_topo/core/Visualizer.hpp"

// I/O functionality
#include "gvd_topo/io/YamlLoader.hpp"

// Utilities
#include "gvd_topo/utils/Timer.hpp"
#include "gvd_topo/utils/ConfigManager.hpp"

// CLI functionality
#include "gvd_topo/cli/CliApplication.hpp"

// ROS adapters (if available)
#include "gvd_topo/ros_adapters.hpp"

// Parameters
#include "gvd_topo/parameters.hpp"

/**
 * @namespace gvd_topo
 * @brief Main namespace for the General Voronoi Diagram to Topological Map library
 * 
 * This namespace contains all the classes and functions provided by the library
 * for generating topological maps from occupancy grid maps using Generalized
 * Voronoi Diagrams (GVD).
 */
