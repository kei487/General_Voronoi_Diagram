#include "gvd_topo/utils/ConfigManager.hpp"
#include <iostream>
#include <stdexcept>

namespace gvd_topo {

ConfigManager::ConfigManager() : config_() {
    // Initialize with default values
}

bool ConfigManager::parseCommandLine(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--config" && i + 1 < argc) {
            config_file_path_ = argv[++i];
        }
        else if ((arg == "--input" || arg == "-i") && i + 1 < argc) {
            config_.input.image_file = argv[++i];
        }
        else if ((arg == "--yaml" || arg == "-y") && i + 1 < argc) {
            config_.input.yaml_file = argv[++i];
        }
        else if (arg == "--resolution" && i + 1 < argc) {
            config_.input.resolution = std::stod(argv[++i]);
        }
        else if (arg == "--occ-thresh" && i + 1 < argc) {
            config_.input.occupancy_threshold = std::stoi(argv[++i]);
        }
        else if (arg == "--out-map" && i + 1 < argc) {
            config_.output.map_file = argv[++i];
        }
        else if (arg == "--out-gvd" && i + 1 < argc) {
            config_.output.gvd_image = argv[++i];
        }
        else if (arg == "--out-topo-image" && i + 1 < argc) {
            config_.output.topo_image = argv[++i];
        }
        else if (arg == "--bench-w" && i + 1 < argc) {
            config_.benchmark.width = std::stoi(argv[++i]);
            config_.benchmark.enabled = true;
        }
        else if (arg == "--bench-h" && i + 1 < argc) {
            config_.benchmark.height = std::stoi(argv[++i]);
            config_.benchmark.enabled = true;
        }
        else if (arg == "--bench-occ" && i + 1 < argc) {
            config_.benchmark.occupancy_ratio = std::stod(argv[++i]);
            config_.benchmark.enabled = true;
        }
        else if (arg == "--seed" && i + 1 < argc) {
            config_.benchmark.seed = static_cast<unsigned>(std::stoul(argv[++i]));
        }
        else if (arg == "--help" || arg == "-h") {
            printHelp(argv[0]);
            return false; // Indicate that help was shown
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return false;
        }
    }
    
    return true;
}

bool ConfigManager::loadFromFile(const std::string& config_path) {
    try {
        ConfigOptions file_config = YamlLoader::loadConfigFromYaml(config_path);
        mergeConfigurations(file_config);
        config_file_path_ = config_path;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigManager::saveToFile(const std::string& config_path) const {
    try {
        YamlLoader::saveConfigToYaml(config_, config_path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving config file: " << e.what() << std::endl;
        return false;
    }
}

void ConfigManager::printHelp(const std::string& program_name) const {
    std::cout << "Usage: " << program_name << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --config <file>        Load configuration from YAML file\n";
    std::cout << "  --input, -i <file>     Input image file\n";
    std::cout << "  --yaml, -y <file>      Input YAML map file\n";
    std::cout << "  --resolution <val>     Resolution (meters per pixel)\n";
    std::cout << "  --occ-thresh <val>     Occupancy threshold\n";
    std::cout << "  --out-map <file>       Output map file\n";
    std::cout << "  --out-gvd <file>       Output GVD image\n";
    std::cout << "  --out-topo-image <file> Output topology image\n";
    std::cout << "  --bench-w <val>        Benchmark width\n";
    std::cout << "  --bench-h <val>        Benchmark height\n";
    std::cout << "  --bench-occ <val>      Benchmark occupancy ratio\n";
    std::cout << "  --seed <val>           Random seed\n";
    std::cout << "  --help, -h             Show this help\n";
}

bool ConfigManager::isValid() const {
    // Check if at least one input source is specified
    if (config_.input.image_file.empty() && 
        config_.input.yaml_file.empty() && 
        !config_.benchmark.enabled) {
        return false;
    }
    
    // Check if resolution is positive
    if (config_.input.resolution <= 0.0) {
        return false;
    }
    
    // Check if occupancy threshold is valid
    if (config_.input.occupancy_threshold < 0 || config_.input.occupancy_threshold > 100) {
        return false;
    }
    
    return true;
}

void ConfigManager::mergeConfigurations(const ConfigOptions& file_config) {
    // Command line arguments override file configuration
    if (config_.input.image_file.empty()) {
        config_.input.image_file = file_config.input.image_file;
    }
    if (config_.input.yaml_file.empty()) {
        config_.input.yaml_file = file_config.input.yaml_file;
    }
    if (config_.input.resolution == 0.05) { // Default value
        config_.input.resolution = file_config.input.resolution;
    }
    if (config_.input.occupancy_threshold == 50) { // Default value
        config_.input.occupancy_threshold = file_config.input.occupancy_threshold;
    }
    if (config_.output.map_file.empty()) {
        config_.output.map_file = file_config.output.map_file;
    }
    if (config_.output.gvd_image.empty()) {
        config_.output.gvd_image = file_config.output.gvd_image;
    }
    if (config_.output.topo_image.empty()) {
        config_.output.topo_image = file_config.output.topo_image;
    }
    if (!config_.benchmark.enabled) {
        config_.benchmark = file_config.benchmark;
    }
    config_.processing = file_config.processing;
}

} // namespace gvd_topo

