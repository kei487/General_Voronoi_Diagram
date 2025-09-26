# Generalized Voronoi Diagram to Topological Map

A C++ library for generating topological maps from occupancy grid maps using Generalized Voronoi Diagrams (GVD).

## Features

- **Occupancy Grid Processing**: Load PNG/PGM images and ROS-compatible occupancy grids
- **GVD Generation**: Extract skeleton using Euclidean Distance Transform and ridge detection
- **Topology Extraction**: Identify nodes (junctions/endpoints) and edges from GVD
- **ROS-Ready**: Message adapters and parameter structures for ROS integration
- **Performance Optimized**: OpenMP parallelization and native optimizations
- **Multiple Output Formats**: JSON topological maps and PNG visualization overlays

## Build Requirements

- C++17 compiler
- CMake 3.16+
- OpenCV (optional, for image I/O and distance transform)
- OpenMP (optional, for parallelization)

## Quick Start

```bash
# Build
cmake -B build -S .
cmake --build build -j

# Run on sample map
./build/bin/gvd_topo_cli --input data/room.pgm --resolution 0.05 \
  --out-map room_topo.json --out-gvd room_gvd.png
```

## Usage

### Command Line Interface

```bash
./build/bin/gvd_topo_cli [OPTIONS]

Options:
  --input, -i <path>        Input image file (PNG/PGM)
  --resolution <float>      Map resolution in meters/pixel (default: 0.05)
  --occ-thresh <int>        Occupancy threshold 0-100 (default: 50)
  --out-map <path>          Output topological map JSON
  --out-gvd <path>          Output GVD overlay PNG
  --bench-w <int>           Benchmark mode: synthetic map width
  --bench-h <int>           Benchmark mode: synthetic map height
  --bench-occ <float>       Benchmark mode: obstacle ratio 0.0-1.0
```

### Library Usage

```cpp
#include "gvd_topo/OccupancyGrid.hpp"
#include "gvd_topo/GvdGenerator.hpp"
#include "gvd_topo/TopologyExtractor.hpp"

// Load occupancy grid
auto grid = OccupancyGrid::loadFromImage("map.pgm", 0.05, 50);

// Generate GVD
GvdGenerator gvd;
auto gvd_result = gvd.run(grid);

// Extract topology
TopologyExtractor topo;
auto topological_map = topo.run(gvd_result.gvd_mask, 
                               gvd_result.width, gvd_result.height, 0.05);
```

## Test Data

Synthetic test maps are available in `data/`:
- `room.pgm`: Room with door (150x150)
- `corridor.pgm`: Corridor with obstacle (200x100)  
- `t_junction.pgm`: T-junction layout (200x200)

## Output Format

Topological maps are saved as JSON:

```json
{
  "nodes": [
    {"id": 0, "x": 3.75, "y": 3.75},
    {"id": 1, "x": 7.25, "y": 2.85}
  ],
  "edges": [
    {"id": 0, "u": 0, "v": 1, "length": 4.2, "polyline": [[3.75,3.75], [7.25,2.85]]}
  ]
}
```

## Performance

Benchmark results on 1000x1000 synthetic map:
- Load+Preprocess: ~170ms
- EDT+GVD: ~110ms  
- Topology: varies by complexity

## ROS Integration

The library provides ROS-compatible data structures:

```cpp
#include "gvd_topo/ros_adapters.hpp"
#include "gvd_topo/parameters.hpp"

// Convert to ROS message format
OccupancyGridMsg msg = toMsg(occupancy_grid);
OccupancyGrid grid = fromMsg(msg);

// Generate visualization markers
MarkerArray markers = topologicalMapToMarkers(topological_map);

// Parameter management
ProcessingParameters params = getDefaultProcessingParameters();
```

## License

See LICENSE file for details.
