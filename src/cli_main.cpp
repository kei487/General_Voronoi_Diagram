#include <iostream>
#include <string>
#include <fstream>
#include "gvd_topo/OccupancyGrid.hpp"
#include "gvd_topo/GvdGenerator.hpp"
#include "gvd_topo/TopologyExtractor.hpp"
#include "gvd_topo/Timer.hpp"

#ifdef GVD_TOPO_WITH_OPENCV
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

int main(int argc, char** argv) {
    using namespace gvd_topo;
    std::string input;
    std::string out_map;
    std::string out_gvd;
    double resolution = 0.05;
    int occ_threshold = 50;
    int bench_w = 0, bench_h = 0; double bench_occ = 0.3; unsigned seed = 12345;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "--input" || a == "-i") && i + 1 < argc) { input = argv[++i]; }
        else if (a == "--resolution" && i + 1 < argc) { resolution = std::stod(argv[++i]); }
        else if (a == "--occ-thresh" && i + 1 < argc) { occ_threshold = std::stoi(argv[++i]); }
        else if (a == "--out-map" && i + 1 < argc) { out_map = argv[++i]; }
        else if (a == "--out-gvd" && i + 1 < argc) { out_gvd = argv[++i]; }
        else if (a == "--bench-w" && i + 1 < argc) { bench_w = std::stoi(argv[++i]); }
        else if (a == "--bench-h" && i + 1 < argc) { bench_h = std::stoi(argv[++i]); }
        else if (a == "--bench-occ" && i + 1 < argc) { bench_occ = std::stod(argv[++i]); }
        else if (a == "--seed" && i + 1 < argc) { seed = static_cast<unsigned>(std::stoul(argv[++i])); }
    }

    auto report = [](const std::string& label, double ms){ std::cout << label << ": " << ms << " ms" << std::endl; };

    OccupancyGrid grid;
    {
        ScopeTimer t("load+preprocess", report);
        if (bench_w > 0 && bench_h > 0) {
            grid = OccupancyGrid::randomMap(bench_w, bench_h, resolution, bench_occ, seed);
        } else if (!input.empty()) {
            try { grid = OccupancyGrid::loadFromImage(input, resolution, occ_threshold); }
            catch (const std::exception& e) { std::cerr << e.what() << std::endl; return 1; }
        } else {
            grid = OccupancyGrid(10, 10, resolution);
        }
    }

    GvdResult res;
    {
        ScopeTimer t("EDT+GVD", report);
        GvdGenerator gvd;
        res = gvd.run(grid);
    }

    TopologicalMap map;
    {
        ScopeTimer t("topology", report);
        TopologyExtractor topo;
        map = topo.run(res.gvd_mask, res.width, res.height, resolution);
    }

    size_t gvd_count = 0; for (auto v : res.gvd_mask) if (v) ++gvd_count;

    std::cout << "gvd_topo_cli: (" << res.width << "x" << res.height << ")" << std::endl;
    std::cout << "GVD pixels=" << gvd_count << std::endl;
    std::cout << "nodes=" << map.nodes.size() << ", edges=" << map.edges.size() << std::endl;

    if (!out_map.empty()) {
        std::ofstream ofs(out_map);
        if (!ofs) { std::cerr << "Failed to open " << out_map << std::endl; return 1; }
        ofs << toJson(map) << std::endl;
        std::cout << "Wrote map: " << out_map << std::endl;
    }

#ifdef GVD_TOPO_WITH_OPENCV
    if (!out_gvd.empty()) {
        ScopeTimer t("overlay", report);
        cv::Mat vis(grid.height, grid.width, CV_8UC3);
        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                int8_t c = grid.data[grid.index(x, y)];
                uint8_t v = 127;
                if (c == static_cast<int8_t>(100)) v = 0; else if (c == static_cast<int8_t>(0)) v = 255;
                vis.at<cv::Vec3b>(y,x) = cv::Vec3b(v,v,v);
            }
        }
        for (int y = 0; y < res.height; ++y) {
            for (int x = 0; x < res.width; ++x) {
                if (res.gvd_mask[y * res.width + x]) vis.at<cv::Vec3b>(y,x) = cv::Vec3b(255,0,0);
            }
        }
        for (const auto& e : map.edges) {
            for (size_t i = 1; i < e.polyline.size(); ++i) {
                cv::Point p0(static_cast<int>(std::round(e.polyline[i-1].first / resolution)),
                             static_cast<int>(std::round(e.polyline[i-1].second / resolution)));
                cv::Point p1(static_cast<int>(std::round(e.polyline[i].first / resolution)),
                             static_cast<int>(std::round(e.polyline[i].second / resolution)));
                cv::line(vis, p0, p1, cv::Scalar(0,0,255), 1);
            }
        }
        for (const auto& n : map.nodes) {
            cv::Point p(static_cast<int>(std::round(n.x / resolution)),
                        static_cast<int>(std::round(n.y / resolution)));
            cv::circle(vis, p, 2, cv::Scalar(0,255,0), -1);
        }
        if (!cv::imwrite(out_gvd, vis)) { std::cerr << "Failed to write " << out_gvd << std::endl; return 1; }
        std::cout << "Wrote overlay: " << out_gvd << std::endl;
    }
#endif

    return 0;
}


