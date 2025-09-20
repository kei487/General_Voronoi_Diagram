#include "gvd_topo/parameters.hpp"

namespace gvd_topo {

ProcessingParameters getDefaultProcessingParameters() {
    ProcessingParameters params;
    // All values are initialized in the struct definition
    return params;
}

NodeParameters getDefaultNodeParameters() {
    NodeParameters params;
    // All values are initialized in the struct definition
    return params;
}

bool validateParameters(const ProcessingParameters& params) {
    if (params.occupancy_threshold < 0 || params.occupancy_threshold > 100) {
        return false;
    }
    if (params.prune_min_length < 0.0) {
        return false;
    }
    if (params.merge_radius < 0.0) {
        return false;
    }
    if (params.max_trace_steps <= 0) {
        return false;
    }
    if (params.morphology_kernel_size < 0) {
        return false;
    }
    return true;
}

bool validateParameters(const NodeParameters& params) {
    if (!validateParameters(params.processing)) {
        return false;
    }
    if (params.map_topic.empty()) {
        return false;
    }
    if (params.topological_map_topic.empty()) {
        return false;
    }
    if (params.markers_topic.empty()) {
        return false;
    }
    if (params.generate_service.empty()) {
        return false;
    }
    return true;
}

} // namespace gvd_topo
