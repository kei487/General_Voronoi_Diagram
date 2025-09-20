#include "gvd_topo/TopologyExtractor.hpp"
#include <cstdint>
#include <queue>
#include <cmath>
#include <limits>
#include <sstream>

namespace gvd_topo {

TopologyExtractor::TopologyExtractor() = default;
TopologyExtractor::TopologyExtractor(const Params& p) : params_(p) {}

static inline int idx(int x, int y, int w) { return y * w + x; }

TopologicalMap TopologyExtractor::run(const std::vector<uint8_t>& gvd_mask, int width, int height, double resolution) const {
    TopologicalMap topo;
    if (gvd_mask.empty() || width <= 0 || height <= 0) return topo;

    auto inBounds = [&](int x, int y) { return x >= 0 && y >= 0 && x < width && y < height; };

    // Compute degrees for each skeleton pixel (8-neighborhood)
    std::vector<uint8_t> is_skel = gvd_mask;
    std::vector<uint8_t> degree(static_cast<size_t>(width * height), 0);
    const int dx8[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    const int dy8[8] = { -1,-1,-1,  0, 0,  1, 1, 1 };
    #ifdef GVD_TOPO_WITH_OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!is_skel[idx(x,y,width)]) continue;
            int deg = 0;
            for (int k = 0; k < 8; ++k) {
                int nx = x + dx8[k];
                int ny = y + dy8[k];
                if (inBounds(nx,ny) && is_skel[idx(nx,ny,width)]) ++deg;
            }
            degree[idx(x,y,width)] = static_cast<uint8_t>(deg);
        }
    }

    // Identify raw nodes (endpoints degree==1, junctions degree>=3)
    struct NodePix { int x; int y; };
    std::vector<NodePix> raw_nodes;
    raw_nodes.reserve(1024);
    {
        std::vector<NodePix> local_nodes;
        local_nodes.reserve(1024);
        #ifdef GVD_TOPO_WITH_OPENMP
        #pragma omp parallel private(local_nodes)
        {
            local_nodes.clear();
            #pragma omp for nowait schedule(static)
            for (int y = 1; y < height-1; ++y) {
                for (int x = 1; x < width-1; ++x) {
                    if (!is_skel[idx(x,y,width)]) continue;
                    int d = degree[idx(x,y,width)];
                    if (d == 1 || d >= 3) local_nodes.push_back({x,y});
                }
            }
            #pragma omp critical
            raw_nodes.insert(raw_nodes.end(), local_nodes.begin(), local_nodes.end());
        }
        #else
        for (int y = 1; y < height-1; ++y) {
            for (int x = 1; x < width-1; ++x) {
                if (!is_skel[idx(x,y,width)]) continue;
                int d = degree[idx(x,y,width)];
                if (d == 1 || d >= 3) raw_nodes.push_back({x,y});
            }
        }
        #endif
    }

    // Merge nearby nodes within merge_radius (pixels)
    const double merge_radius_px = params_.merge_radius / resolution;
    const double merge_radius_px2 = merge_radius_px * merge_radius_px;
    std::vector<int> parent(raw_nodes.size());
    for (size_t i = 0; i < parent.size(); ++i) parent[i] = static_cast<int>(i);
    auto findp = [&](int a){ while (parent[a] != a) a = parent[a] = parent[parent[a]]; return a; };
    auto un = [&](int a, int b){ a = findp(a); b = findp(b); if (a!=b) parent[b]=a; };
    for (size_t i = 0; i < raw_nodes.size(); ++i) {
        for (size_t j = i+1; j < raw_nodes.size(); ++j) {
            double dx = static_cast<double>(raw_nodes[i].x - raw_nodes[j].x);
            double dy = static_cast<double>(raw_nodes[i].y - raw_nodes[j].y);
            if (dx*dx + dy*dy <= merge_radius_px2) un(static_cast<int>(i), static_cast<int>(j));
        }
    }
    // Compute representatives and average positions
    std::vector<std::vector<int>> groups(raw_nodes.size());
    for (size_t i = 0; i < raw_nodes.size(); ++i) groups[findp(static_cast<int>(i))].push_back(static_cast<int>(i));
    std::vector<int> label(width*height, -1);
    int node_id = 0;
    for (const auto& g : groups) if (!g.empty()) {
        double sx=0, sy=0; for (int id : g){ sx += raw_nodes[id].x; sy += raw_nodes[id].y; }
        int cx = static_cast<int>(std::round(sx / static_cast<double>(g.size())));
        int cy = static_cast<int>(std::round(sy / static_cast<double>(g.size())));
        TopoNode n; n.id = node_id; n.x = cx * resolution; n.y = cy * resolution;
        topo.nodes.push_back(n);
        label[idx(cx,cy,width)] = node_id;
        ++node_id;
    }

    // Edge tracing: from each node, follow skeleton until another node or endpoint
    std::vector<uint8_t> visited(width*height, 0);
    auto isNode = [&](int x, int y){ int id = label[idx(x,y,width)]; return id >= 0; };

    for (const auto& n : topo.nodes) {
        int sx = static_cast<int>(std::round(n.x / resolution));
        int sy = static_cast<int>(std::round(n.y / resolution));
        for (int k = 0; k < 8; ++k) {
            int nx = sx + dx8[k];
            int ny = sy + dy8[k];
            if (!inBounds(nx,ny) || !is_skel[idx(nx,ny,width)]) continue;
            if (visited[idx(nx,ny,width)]) continue;
            // trace path
            std::vector<std::pair<double,double>> poly;
            int px = sx; int py = sy; int cx = nx; int cy = ny;
            double length = 0.0;
            int steps = 0;
            while (steps < 2 * width * height) {
                visited[idx(cx,cy,width)] = 1;
                poly.emplace_back(cx * resolution, cy * resolution);
                if (isNode(cx,cy) && !(cx == sx && cy == sy)) {
                    int to_id = label[idx(cx,cy,width)];
                    TopoEdge e; e.id = static_cast<int>(topo.edges.size()); e.u = n.id; e.v = to_id; e.length = length; e.polyline = poly;
                    topo.edges.push_back(std::move(e));
                    break;
                }
                // prevent cycling back to start node
                if (cx == sx && cy == sy) {
                    break;
                }
                // choose next neighbor (avoid returning to previous pixel)
                int nextx = -1, nexty = -1, choices = 0;
                for (int kk = 0; kk < 8; ++kk) {
                    int tx = cx + dx8[kk];
                    int ty = cy + dy8[kk];
                    if (!inBounds(tx,ty) || !is_skel[idx(tx,ty,width)]) continue;
                    if (tx == px && ty == py) continue;
                    if (visited[idx(tx,ty,width)]) continue; // avoid cycles
                    ++choices; nextx = tx; nexty = ty;
                }
                if (choices == 0) {
                    // dead end -> create node if not exist
                    if (!isNode(cx,cy)) {
                        TopoNode m; m.id = node_id; m.x = cx * resolution; m.y = cy * resolution; topo.nodes.push_back(m); label[idx(cx,cy,width)] = node_id; ++node_id;
                        int to_id = label[idx(cx,cy,width)];
                        TopoEdge e; e.id = static_cast<int>(topo.edges.size()); e.u = n.id; e.v = to_id; e.length = length; e.polyline = poly;
                        topo.edges.push_back(std::move(e));
                    }
                    break;
                }
                length += std::hypot(static_cast<double>(nextx - cx) * resolution, static_cast<double>(nexty - cy) * resolution);
                px = cx; py = cy; cx = nextx; cy = nexty;
                ++steps;
            }
        }
    }

    // Pruning: remove edges shorter than threshold
    const double min_len = params_.prune_min_length;
    std::vector<TopoEdge> kept;
    kept.reserve(topo.edges.size());
    for (auto& e : topo.edges) if (e.length >= min_len) kept.push_back(std::move(e));
    topo.edges.swap(kept);

    return topo;
}

std::string toJson(const TopologicalMap& map) {
    std::ostringstream os;
    os << "{\n  \"nodes\": [\n";
    for (size_t i = 0; i < map.nodes.size(); ++i) {
        const auto& n = map.nodes[i];
        os << "    {\"id\": " << n.id << ", \"x\": " << n.x << ", \"y\": " << n.y << "}";
        if (i + 1 < map.nodes.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n  \"edges\": [\n";
    for (size_t i = 0; i < map.edges.size(); ++i) {
        const auto& e = map.edges[i];
        os << "    {\"id\": " << e.id << ", \"u\": " << e.u << ", \"v\": " << e.v << ", \"length\": " << e.length << ", \"polyline\": [";
        for (size_t j = 0; j < e.polyline.size(); ++j) {
            os << "[" << e.polyline[j].first << ", " << e.polyline[j].second << "]";
            if (j + 1 < e.polyline.size()) os << ", ";
        }
        os << "]}";
        if (i + 1 < map.edges.size()) os << ",";
        os << "\n";
    }
    os << "  ]\n}";
    return os.str();
}

} // namespace gvd_topo


