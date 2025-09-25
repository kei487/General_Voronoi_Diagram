#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace gvd_topo {

enum class Cell : int8_t {
    Unknown = -1,
    Free = 0,
    Occupied = 100
};

struct Origin2D {
    double x {0.0};
    double y {0.0};
    double theta {0.0};
};

class OccupancyGrid {
public:
    int width {0};
    int height {0};
    double resolution {0.05};
    Origin2D origin {};
    std::vector<int8_t> data; // row-major, size == width*height

    OccupancyGrid() = default;
    OccupancyGrid(int w, int h, double res);

    bool empty() const { return width <= 0 || height <= 0 || data.empty(); }
    int index(int x, int y) const { return y * width + x; }
    bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < width && y < height; }

    // I/O helpers (OpenCV if available)
    static OccupancyGrid loadFromImage(const std::string& path, double resolution, int occ_threshold);
    void saveDebugPGM(const std::string& path) const;

    // Benchmark helper
    static OccupancyGrid randomMap(int w, int h, double res, double occupied_ratio, uint32_t seed = 12345);
};

} // namespace gvd_topo


