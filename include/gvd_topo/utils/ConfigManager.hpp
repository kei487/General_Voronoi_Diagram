#pragma once

#include "gvd_topo/io/YamlLoader.hpp"
#include <string>
#include <memory>

namespace gvd_topo {

/**
 * @brief Configuration management class
 * 
 * This class provides a unified interface for managing configuration
 * from both command line arguments and YAML files.
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager() = default;

    /**
     * @brief Parse command line arguments
     * @param argc Argument count
     * @param argv Argument vector
     * @return true if parsing successful, false otherwise
     */
    bool parseCommandLine(int argc, char** argv);

    /**
     * @brief Load configuration from YAML file
     * @param config_path Path to YAML configuration file
     * @return true if loading successful, false otherwise
     */
    bool loadFromFile(const std::string& config_path);

    /**
     * @brief Get current configuration
     * @return Reference to current configuration
     */
    const ConfigOptions& getConfig() const { return config_; }

    /**
     * @brief Set configuration
     * @param config New configuration
     */
    void setConfig(const ConfigOptions& config) { config_ = config; }

    /**
     * @brief Save current configuration to YAML file
     * @param config_path Path to save configuration
     * @return true if saving successful, false otherwise
     */
    bool saveToFile(const std::string& config_path) const;

    /**
     * @brief Print help message
     * @param program_name Name of the program
     */
    void printHelp(const std::string& program_name) const;

    /**
     * @brief Check if configuration is valid
     * @return true if valid, false otherwise
     */
    bool isValid() const;

private:
    ConfigOptions config_;
    std::string config_file_path_;
    
    /**
     * @brief Merge file configuration with command line configuration
     * @param file_config Configuration from file
     */
    void mergeConfigurations(const ConfigOptions& file_config);
};

} // namespace gvd_topo
