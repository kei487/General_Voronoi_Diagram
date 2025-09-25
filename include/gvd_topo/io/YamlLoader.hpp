#pragma once

#include "gvd_topo/core/OccupancyGrid.hpp"
#include <string>

namespace gvd_topo {

struct MapInfo {
    std::string image_file;
    double resolution;
    double origin_x, origin_y, origin_theta;
    double occupied_thresh;
    double free_thresh;
    bool negate;
};

// Configuration structure for command-line options
struct ConfigOptions {
    // Input configuration
    struct InputConfig {
        std::string image_file;
        std::string yaml_file;
        double resolution = 0.05;
        int occupancy_threshold = 50;
    } input;
    
    // Output configuration
    struct OutputConfig {
        std::string map_file;
        std::string gvd_image;
        std::string topo_image;
    } output;
    
    // Benchmark configuration
    struct BenchmarkConfig {
        bool enabled = false;
        int width = 100;
        int height = 100;
        double occupancy_ratio = 0.3;
        unsigned seed = 12345;
    } benchmark;
    
    // Processing parameters
    struct ProcessingConfig {
        double distance_epsilon = 1e-6;
        bool use_opencv = true;
        double prune_min_length = 0.5;
        double merge_radius = 0.2;
        int max_trace_steps = 100000;
        bool use_parallel_processing = true;
        int max_threads = 0; // 0 = auto-detect
    } processing;
};

class YamlLoader {
public:
    static MapInfo loadMapInfo(const std::string& yaml_path);
    static OccupancyGrid loadFromYaml(const std::string& yaml_path, int occ_threshold = 50);
    
    // New methods for configuration file loading
    static ConfigOptions loadConfigFromYaml(const std::string& config_path);
    static void saveConfigToYaml(const ConfigOptions& config, const std::string& config_path);
};

} // namespace gvd_topo


