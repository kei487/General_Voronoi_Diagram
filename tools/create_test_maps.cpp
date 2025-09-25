#include "gvd_topo/core/OccupancyGrid.hpp"
#include <iostream>
#include <fstream>

#ifdef GVD_TOPO_WITH_OPENCV
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

namespace gvd_topo {

// Create a simple corridor map
OccupancyGrid createCorridorMap() {
    const int w = 200, h = 100;
    OccupancyGrid grid(w, h, 0.05);
    
    // Fill with free space
    std::fill(grid.data.begin(), grid.data.end(), static_cast<int8_t>(0));
    
    // Add walls on top and bottom
    for (int x = 0; x < w; ++x) {
        grid.data[grid.index(x, 0)] = static_cast<int8_t>(100); // top wall
        grid.data[grid.index(x, h-1)] = static_cast<int8_t>(100); // bottom wall
    }
    
    // Add vertical obstacle in the middle
    for (int y = 20; y < 80; ++y) {
        grid.data[grid.index(w/2, y)] = static_cast<int8_t>(100);
    }
    
    return grid;
}

// Create a room with door map
OccupancyGrid createRoomMap() {
    const int w = 150, h = 150;
    OccupancyGrid grid(w, h, 0.05);
    
    // Fill with free space
    std::fill(grid.data.begin(), grid.data.end(), static_cast<int8_t>(0));
    
    // Outer walls
    for (int x = 0; x < w; ++x) {
        grid.data[grid.index(x, 0)] = static_cast<int8_t>(100); // top
        grid.data[grid.index(x, h-1)] = static_cast<int8_t>(100); // bottom
    }
    for (int y = 0; y < h; ++y) {
        grid.data[grid.index(0, y)] = static_cast<int8_t>(100); // left
        grid.data[grid.index(w-1, y)] = static_cast<int8_t>(100); // right
    }
    
    // Door in the middle of top wall
    grid.data[grid.index(w/2, 0)] = static_cast<int8_t>(0);
    grid.data[grid.index(w/2-1, 0)] = static_cast<int8_t>(0);
    grid.data[grid.index(w/2+1, 0)] = static_cast<int8_t>(0);
    
    return grid;
}

// Create a T-junction map
OccupancyGrid createTJunctionMap() {
    const int w = 200, h = 200;
    OccupancyGrid grid(w, h, 0.05);
    
    // Fill with free space
    std::fill(grid.data.begin(), grid.data.end(), static_cast<int8_t>(0));
    
    // Horizontal corridor
    for (int x = 20; x < w-20; ++x) {
        grid.data[grid.index(x, h/2)] = static_cast<int8_t>(100);
        grid.data[grid.index(x, h/2-1)] = static_cast<int8_t>(100);
        grid.data[grid.index(x, h/2+1)] = static_cast<int8_t>(100);
    }
    
    // Vertical corridor
    for (int y = 20; y < h-20; ++y) {
        grid.data[grid.index(w/2, y)] = static_cast<int8_t>(100);
        grid.data[grid.index(w/2-1, y)] = static_cast<int8_t>(100);
        grid.data[grid.index(w/2+1, y)] = static_cast<int8_t>(100);
    }
    
    return grid;
}

} // namespace gvd_topo

int main() {
    using namespace gvd_topo;
    
    std::cout << "Creating test maps..." << std::endl;
    
    // Create test maps
    auto corridor = createCorridorMap();
    auto room = createRoomMap();
    auto t_junction = createTJunctionMap();
    
    // Save as PGM files
    corridor.saveDebugPGM("/home/keitaro/cursor_ws/General_Voronoi_Diagram/data/corridor.pgm");
    room.saveDebugPGM("/home/keitaro/cursor_ws/General_Voronoi_Diagram/data/room.pgm");
    t_junction.saveDebugPGM("/home/keitaro/cursor_ws/General_Voronoi_Diagram/data/t_junction.pgm");
    
    std::cout << "Created test maps:" << std::endl;
    std::cout << "- corridor.pgm (" << corridor.width << "x" << corridor.height << ")" << std::endl;
    std::cout << "- room.pgm (" << room.width << "x" << room.height << ")" << std::endl;
    std::cout << "- t_junction.pgm (" << t_junction.width << "x" << t_junction.height << ")" << std::endl;
    
    return 0;
}
