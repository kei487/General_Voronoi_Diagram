#pragma once

#include "gvd_topo/OccupancyGrid.hpp"
#include "gvd_topo/TopologyExtractor.hpp"
#include <vector>

namespace gvd_topo {

// ROS message-like structures (no ROS dependency)
struct Pose2D {
    double x {0.0};
    double y {0.0};
    double theta {0.0};
};

struct OccupancyGridMsg {
    int width {0};
    int height {0};
    double resolution {0.05};
    Pose2D origin {};
    std::vector<int8_t> data;
};

struct Marker {
    int id {0};
    Pose2D pose {};
    double scale {1.0};
    std::vector<uint8_t> color {255, 0, 0, 255}; // RGBA
};

struct MarkerArray {
    std::vector<Marker> markers;
};

// Adapter functions
OccupancyGrid fromMsg(const OccupancyGridMsg& msg);
OccupancyGridMsg toMsg(const OccupancyGrid& grid);

MarkerArray topologicalMapToMarkers(const TopologicalMap& map, const std::string& frame_id = "map");
std::vector<Marker> nodesToMarkers(const std::vector<TopoNode>& nodes, const std::string& frame_id = "map");
std::vector<Marker> edgesToMarkers(const std::vector<TopoEdge>& edges, const std::string& frame_id = "map");

} // namespace gvd_topo
