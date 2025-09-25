#include <iostream>
#include "gvd_topo/gvd_topo.hpp"

int main(int argc, char** argv) {
    using namespace gvd_topo;
    
    // Initialize configuration manager
    ConfigManager config_manager;
    
    // Parse command line arguments
    if (!config_manager.parseCommandLine(argc, argv)) {
        return 0; // Help was shown
    }
    
    // Load configuration from file if specified
    if (!config_manager.getConfig().input.image_file.empty() || 
        !config_manager.getConfig().input.yaml_file.empty()) {
        // Configuration already loaded from command line
    } else {
        // Try to load from default config file
        std::string default_config = "data/example_config.yaml";
        if (!config_manager.loadFromFile(default_config)) {
            std::cerr << "No input specified. Use --help for usage information." << std::endl;
            return 1;
        }
    }
    
    // Validate configuration
    if (!config_manager.isValid()) {
        std::cerr << "Invalid configuration. Use --help for usage information." << std::endl;
        return 1;
    }
    
    // Create and run application
    CliApplication app;
    
    // Set up timing callback
    app.setTimingCallback([](const std::string& label, double ms) {
        std::cout << label << ": " << ms << " ms" << std::endl;
    });
    
    // Run the application
    return app.run(config_manager.getConfig());
}


