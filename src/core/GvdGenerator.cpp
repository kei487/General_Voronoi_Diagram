#include "gvd_topo/core/GvdGenerator.hpp"
#include "gvd_topo/core/OccupancyGrid.hpp"
#ifdef GVD_TOPO_WITH_OPENCV
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#endif

namespace gvd_topo {

GvdGenerator::GvdGenerator() = default;
GvdGenerator::GvdGenerator(const Params& p) : params_(p) {}

GvdResult GvdGenerator::run(const OccupancyGrid& grid) const {
    GvdResult result;
    result.width = grid.width;
    result.height = grid.height;
    result.distance.assign(static_cast<size_t>(grid.width * grid.height), 0.0f);
    result.gvd_mask.assign(static_cast<size_t>(grid.width * grid.height), 0);
    // Simple EDT via OpenCV if available
#ifdef GVD_TOPO_WITH_OPENCV
    if (!grid.empty()) {
        cv::Mat occ(grid.height, grid.width, CV_8UC1);
        for (int y = 0; y < grid.height; ++y) {
            uint8_t* row = occ.ptr<uint8_t>(y);
            for (int x = 0; x < grid.width; ++x) {
                int8_t c = grid.data[grid.index(x, y)];
                row[x] = (c == static_cast<int8_t>(100)) ? 0 : 255;
            }
        }
        cv::Mat dist;
        cv::distanceTransform(occ, dist, cv::DIST_L2, 3);
        // copy back
        for (int y = 0; y < grid.height; ++y) {
            const float* row = dist.ptr<float>(y);
            for (int x = 0; x < grid.width; ++x) {
                result.distance[grid.index(x, y)] = row[x] * static_cast<float>(grid.resolution);
            }
        }

        // Ridge detection: 8-neighborhood local maxima on distance map
        const int w = grid.width;
        const int h = grid.height;
        const float eps = 1e-6f;
        #ifdef GVD_TOPO_WITH_OPENMP
        #pragma omp parallel for schedule(static)
        #endif
        for (int y = 1; y < h - 1; ++y) {
            for (int x = 1; x < w - 1; ++x) {
                float c = result.distance[y * w + x];
                if (c <= eps) continue;
                bool is_max = true;
                for (int ddy = -1; ddy <= 1 && is_max; ++ddy) {
                    for (int ddx = -1; ddx <= 1; ++ddx) {
                        if (ddx == 0 && ddy == 0) continue;
                        if (result.distance[(y + ddy) * w + (x + ddx)] > c + eps) { is_max = false; break; }
                    }
                }
                if (is_max) {
                    result.gvd_mask[y * w + x] = 255;
                }
            }
        }
    }
#endif
    return result;
}

} // namespace gvd_topo


