#include "gvd_topo/OccupancyGrid.hpp"
#include <fstream>
#include <stdexcept>
#include <random>

#ifdef GVD_TOPO_WITH_OPENCV
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

namespace gvd_topo {

OccupancyGrid::OccupancyGrid(int w, int h, double res)
    : width(w), height(h), resolution(res), data(static_cast<size_t>(w) * static_cast<size_t>(h), static_cast<int8_t>(-1)) {}

OccupancyGrid OccupancyGrid::loadFromImage(const std::string& path, double res, int occ_threshold) {
#ifdef GVD_TOPO_WITH_OPENCV
    cv::Mat img = cv::imread(path, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        throw std::runtime_error("Failed to load image: " + path);
    }
    OccupancyGrid grid(img.cols, img.rows, res);
    grid.data.resize(static_cast<size_t>(grid.width * grid.height));
    for (int y = 0; y < img.rows; ++y) {
        const uint8_t* row = img.ptr<uint8_t>(y);
        for (int x = 0; x < img.cols; ++x) {
            uint8_t v = row[x];
            grid.data[grid.index(x, y)] = (v <= occ_threshold) ? static_cast<int8_t>(100) : static_cast<int8_t>(0);
        }
    }
    return grid;
#else
    (void)path; (void)res; (void)occ_threshold;
    throw std::runtime_error("OpenCV is not enabled; cannot load image");
#endif
}

void OccupancyGrid::saveDebugPGM(const std::string& path) const {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return;
    ofs << "P5\n" << width << " " << height << "\n255\n";
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int8_t c = data[index(x, y)];
            uint8_t v = 127;
            if (c == static_cast<int8_t>(100)) v = 0; // occupied -> black
            else if (c == static_cast<int8_t>(0)) v = 255; // free -> white
            ofs.put(static_cast<char>(v));
        }
    }
}

OccupancyGrid OccupancyGrid::randomMap(int w, int h, double res, double occupied_ratio, uint32_t seed) {
    if (occupied_ratio < 0.0) occupied_ratio = 0.0;
    if (occupied_ratio > 1.0) occupied_ratio = 1.0;
    OccupancyGrid g(w, h, res);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            double r = uni(rng);
            g.data[g.index(x,y)] = (r < occupied_ratio) ? static_cast<int8_t>(100) : static_cast<int8_t>(0);
        }
    }
    return g;
}

} // namespace gvd_topo


