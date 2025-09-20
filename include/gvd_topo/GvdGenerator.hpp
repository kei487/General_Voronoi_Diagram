#pragma once

#include <vector>
#include <cstdint>

namespace gvd_topo {

class OccupancyGrid;

struct GvdResult {
    int width {0};
    int height {0};
    std::vector<float> distance; // EDT result, row-major
    std::vector<uint8_t> gvd_mask; // 0/255 skeleton mask
};

class GvdGenerator {
public:
    struct Params {
        int occ_threshold {50};
        int morph_kernel {0}; // 0 means none
    };

    GvdGenerator();
    explicit GvdGenerator(const Params& p);

    GvdResult run(const OccupancyGrid& grid) const;

private:
    Params params_;
};

} // namespace gvd_topo


