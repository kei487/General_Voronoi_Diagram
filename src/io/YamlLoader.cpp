#include "gvd_topo/io/YamlLoader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <iostream>

namespace gvd_topo {

MapInfo YamlLoader::loadMapInfo(const std::string& yaml_path) {
    MapInfo info;
    std::ifstream file(yaml_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open YAML file: " + yaml_path);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Remove comments
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        
        // Skip empty lines
        if (line.empty()) continue;
        
        // Parse key-value pairs
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;
        
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (key == "image") {
            info.image_file = value;
        } else if (key == "resolution") {
            info.resolution = std::stod(value);
        } else if (key == "origin") {
            // Parse array format: [-68.946, -101.928, 0.0]
            value.erase(std::remove(value.begin(), value.end(), '['), value.end());
            value.erase(std::remove(value.begin(), value.end(), ']'), value.end());
            
            std::istringstream iss(value);
            std::string token;
            std::getline(iss, token, ',');
            info.origin_x = std::stod(token);
            std::getline(iss, token, ',');
            info.origin_y = std::stod(token);
            std::getline(iss, token, ',');
            info.origin_theta = std::stod(token);
        } else if (key == "occupied_thresh") {
            info.occupied_thresh = std::stod(value);
        } else if (key == "free_thresh") {
            info.free_thresh = std::stod(value);
        } else if (key == "negate") {
            info.negate = (std::stoi(value) != 0);
        }
    }
    
    return info;
}

OccupancyGrid YamlLoader::loadFromYaml(const std::string& yaml_path, int occ_threshold) {
    MapInfo info = loadMapInfo(yaml_path);
    
    // Get directory of YAML file
    std::filesystem::path yaml_file_path(yaml_path);
    std::filesystem::path yaml_dir = yaml_file_path.parent_path();
    std::filesystem::path image_path = yaml_dir / info.image_file;
    
    // Load image using existing functionality
    OccupancyGrid grid = OccupancyGrid::loadFromImage(image_path.string(), info.resolution, occ_threshold);
    
    // Set origin from YAML
    grid.origin.x = info.origin_x;
    grid.origin.y = info.origin_y;
    grid.origin.theta = info.origin_theta;
    
    return grid;
}

ConfigOptions YamlLoader::loadConfigFromYaml(const std::string& config_path) {
    ConfigOptions config;
    std::ifstream file(config_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + config_path);
    }
    
    // Get directory of config file for relative path resolution
    std::filesystem::path config_file_path(config_path);
    std::filesystem::path config_dir = config_file_path.parent_path();
    
    std::string line;
    std::string current_section;
    
    while (std::getline(file, line)) {
        // Remove comments
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        
        // Skip empty lines
        if (line.empty()) continue;
        
        // Check for section headers
        if (line.back() == ':') {
            current_section = line.substr(0, line.length() - 1);
            // Trim whitespace
            current_section.erase(0, current_section.find_first_not_of(" \t"));
            current_section.erase(current_section.find_last_not_of(" \t") + 1);
            continue;
        }
        
        // Parse key-value pairs
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;
        
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Remove surrounding quotes if present
        if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.length() - 2);
        }
        
        // Parse based on current section
        if (current_section == "input") {
            if (key == "image_file") {
                // Convert relative path to absolute path
                if (!value.empty() && !std::filesystem::path(value).is_absolute()) {
                    config.input.image_file = (config_dir / value).string();
                } else {
                    config.input.image_file = value;
                }
            } else if (key == "yaml_file") {
                // Convert relative path to absolute path
                if (!value.empty() && !std::filesystem::path(value).is_absolute()) {
                    config.input.yaml_file = (config_dir / value).string();
                } else {
                    config.input.yaml_file = value;
                }
            } else if (key == "resolution") {
                config.input.resolution = std::stod(value);
            } else if (key == "occupancy_threshold") {
                config.input.occupancy_threshold = std::stoi(value);
            }
        } else if (current_section == "output") {
            if (key == "map_file") {
                config.output.map_file = value;
            } else if (key == "gvd_image") {
                config.output.gvd_image = value;
            } else if (key == "topo_image") {
                config.output.topo_image = value;
            }
        } else if (current_section == "benchmark") {
            if (key == "enabled") {
                config.benchmark.enabled = (value == "true" || value == "1");
            } else if (key == "width") {
                config.benchmark.width = std::stoi(value);
            } else if (key == "height") {
                config.benchmark.height = std::stoi(value);
            } else if (key == "occupancy_ratio") {
                config.benchmark.occupancy_ratio = std::stod(value);
            } else if (key == "seed") {
                config.benchmark.seed = static_cast<unsigned>(std::stoul(value));
            }
        } else if (current_section == "processing") {
            if (key == "distance_epsilon") {
                config.processing.distance_epsilon = std::stod(value);
            } else if (key == "use_opencv") {
                config.processing.use_opencv = (value == "true" || value == "1");
            } else if (key == "prune_min_length") {
                config.processing.prune_min_length = std::stod(value);
            } else if (key == "merge_radius") {
                config.processing.merge_radius = std::stod(value);
            } else if (key == "max_trace_steps") {
                config.processing.max_trace_steps = std::stoi(value);
            } else if (key == "use_parallel_processing") {
                config.processing.use_parallel_processing = (value == "true" || value == "1");
            } else if (key == "max_threads") {
                config.processing.max_threads = std::stoi(value);
            }
        }
    }
    
    return config;
}

void YamlLoader::saveConfigToYaml(const ConfigOptions& config, const std::string& config_path) {
    std::ofstream file(config_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create config file: " + config_path);
    }
    
    file << "# GVD Topology Generator Configuration File\n";
    file << "# This file contains all the command-line options in YAML format\n\n";
    
    // Input configuration
    file << "input:\n";
    file << "  # Input image file path (equivalent to --input/-i)\n";
    file << "  image_file: \"" << config.input.image_file << "\"\n";
    file << "  \n";
    file << "  # YAML map file path (equivalent to --yaml/-y)\n";
    file << "  # If specified, this takes precedence over image_file\n";
    file << "  yaml_file: \"" << config.input.yaml_file << "\"\n";
    file << "  \n";
    file << "  # Resolution in meters per pixel (equivalent to --resolution)\n";
    file << "  resolution: " << config.input.resolution << "\n";
    file << "  \n";
    file << "  # Occupancy threshold (equivalent to --occ-thresh)\n";
    file << "  occupancy_threshold: " << config.input.occupancy_threshold << "\n\n";
    
    // Output configuration
    file << "output:\n";
    file << "  # Output map file path (equivalent to --out-map)\n";
    file << "  map_file: \"" << config.output.map_file << "\"\n";
    file << "  \n";
    file << "  # Output GVD visualization image (equivalent to --out-gvd)\n";
    file << "  gvd_image: \"" << config.output.gvd_image << "\"\n";
    file << "  \n";
    file << "  # Output topological map visualization (equivalent to --out-topo-image)\n";
    file << "  topo_image: \"" << config.output.topo_image << "\"\n\n";
    
    // Benchmark configuration
    file << "benchmark:\n";
    file << "  # Enable benchmark mode (equivalent to --bench-w, --bench-h, --bench-occ, --seed)\n";
    file << "  enabled: " << (config.benchmark.enabled ? "true" : "false") << "\n";
    file << "  \n";
    file << "  # Benchmark map dimensions\n";
    file << "  width: " << config.benchmark.width << "\n";
    file << "  height: " << config.benchmark.height << "\n";
    file << "  \n";
    file << "  # Occupancy ratio for random map generation\n";
    file << "  occupancy_ratio: " << config.benchmark.occupancy_ratio << "\n";
    file << "  \n";
    file << "  # Random seed for reproducible results\n";
    file << "  seed: " << config.benchmark.seed << "\n\n";
    
    // Processing parameters
    file << "processing:\n";
    file << "  # Distance epsilon for GVD generation\n";
    file << "  distance_epsilon: " << config.processing.distance_epsilon << "\n";
    file << "  \n";
    file << "  # Use OpenCV for processing\n";
    file << "  use_opencv: " << (config.processing.use_opencv ? "true" : "false") << "\n";
    file << "  \n";
    file << "  # Topology extraction parameters\n";
    file << "  prune_min_length: " << config.processing.prune_min_length << "\n";
    file << "  merge_radius: " << config.processing.merge_radius << "\n";
    file << "  max_trace_steps: " << config.processing.max_trace_steps << "\n";
    file << "  \n";
    file << "  # Performance settings\n";
    file << "  use_parallel_processing: " << (config.processing.use_parallel_processing ? "true" : "false") << "\n";
    file << "  max_threads: " << config.processing.max_threads << "  # 0 = auto-detect\n";
}

} // namespace gvd_topo


