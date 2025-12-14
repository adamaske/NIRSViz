# NIRSViz

**Advanced 3D Visualization Platform for Functional Near-Infrared Spectroscopy (fNIRS) Data**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3%2B-green.svg)](https://www.opengl.org/)

---

## Overview
What is the goal of this project? By having fNIRS signals with more information 
NIRSViz is a research-grade visualization platform for functional Near-Infrared Spectroscopy (fNIRS) neuroimaging data. It provides real-time 3D visualization of brain activity by mapping optical probe measurements onto anatomical cortex models with unprecedented performance through advanced spatial indexing algorithms.

Ved å bruke en MCX simulering fra hver a kildene kan vi gi en sensitvitet måling per vertex hvor fargen er angitt, eventuelt kan vi også ta i betraktning de indre verdiene av meshet.  

Mesh-Slicing + Visualisering

### What is fNIRS?

Functional Near-Infrared Spectroscopy (fNIRS) is a non-invasive neuroimaging technique that measures brain activity by detecting changes in blood oxygenation using near-infrared light. fNIRS systems use optical probes (sources and detectors) placed on the scalp to measure hemodynamic responses in the cortex.

### Key Features

- ✨ **Real-time 3D Visualization**: Interactive visualization of fNIRS channel projections onto cortical surfaces
- 🚀 **High-Performance Spatial Indexing**: Dual BVH + KD-tree acceleration for 65x faster computation
- 📊 **Multi-Wavelength Support**: Visualize HbO (oxygenated), HbR (deoxygenated), and HbT (total hemoglobin)
- 📁 **SNIRF File Support**: Load standardized fNIRS data format (Shared Near-Infrared Spectroscopy Format)
- 🧠 **Accurate Cortical Mapping**: Ray-based projection from probe channels to cortex surface
- 🎨 **Advanced Rendering**: Modern OpenGL 3.3+ with custom shaders for activity visualization
- 📐 **Flexible Probe Positioning**: 2D and 3D probe transformation and alignment tools
- 🔬 **Scientific Accuracy**: Distance-based influence mapping with configurable falloff parameters

---

## Screenshots

*[TODO: Add screenshots of the main UI, anatomy viewport, probe editor, and projection visualization]*

---

## Quick Start

### System Requirements

- **Operating System**: Windows 10/11 (primary), Linux/macOS (experimental)
- **Compiler**: C++20 compatible (MSVC 2019+, GCC 11+, or Clang 12+)
- **Graphics**: OpenGL 3.3+ compatible GPU
- **CMake**: 3.15 or higher
- **Dependencies**: Boost 1.70+, HDF5, and others (see Installation Guide)

### Installation

1. **Clone the repository**:
   ```bash
   git clone --recursive https://github.com/[username]/NIRSViz.git
   cd NIRSViz
   ```

2. **Install prerequisites** (see [docs/INSTALLATION.md](docs/INSTALLATION.md) for detailed instructions):
   - Boost (1.70+)
   - HDF5 library
   - CMake (3.15+)

3. **Build the project**:
   ```bash
   mkdir build && cd build
   cmake -G "Visual Studio 17 2022" -A x64 ..
   cmake --build . --config Release
   ```

4. **Run NIRSViz**:
   ```bash
   ./Release/NIRSViz.exe
   ```

For detailed setup instructions, see the [Installation Guide](docs/INSTALLATION.md).

---

## Usage

### Basic Workflow

1. **Load SNIRF Data**: File → Open SNIRF file containing your fNIRS measurements
2. **Load Anatomy**: Import cortex and head mesh models (OBJ, FBX, STL, etc.)
3. **Position Probe**: Use the 3D probe editor to align your probe with the anatomy
4. **Configure Projection**: Set influence radius and falloff parameters
5. **Visualize**: Watch real-time brain activity projection on the cortex surface
6. **Analyze**: Scrub through time, select channels, and export results

For comprehensive usage instructions, see the [User Guide](docs/USER_GUIDE.md).

### Keyboard Shortcuts
[TODO: Keyboard shortcuts]

---

## Architecture

NIRSViz is built on a modular, system-based architecture inspired by game engine design:

### Core Systems

| System | Purpose |
|--------|---------|
| **ProjectionSystem** | Computes and renders cortex activity projections with influence mapping |
| **ProbeSystem** | Manages probe visualization and channel-to-cortex ray intersections |
| **AnatomySystem** | Handles anatomical model loading, rendering, and coordinate systems |
| **ChannelSelectorSystem** | Interactive UI for selecting/filtering fNIRS channels |
| **PlottingSystem** | Time-series data visualization and analysis |

### Performance Innovation: Dual Spatial Indexing

NIRSViz achieves real-time performance through a novel dual spatial acceleration structure:

1. **BVH (Bounding Volume Hierarchy)**: Accelerates ray-triangle intersection for finding where channels hit the cortex (O(log n) vs O(n))
2. **KD-Tree**: Enables fast radius queries to find influenced cortex vertices (65x speedup: 17ms vs 1144ms)

This combination allows interactive fNIRS visualization on high-resolution anatomical meshes.

For detailed architecture documentation, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

---

## Technology Stack

NIRSViz integrates cutting-edge C++ libraries:

| Library | Purpose |
|---------|---------|
| **BVH** (madmann91/bvh) | Ray-triangle intersection acceleration with parallel builder |
| **nanoflann** | KD-tree for fast 3D spatial queries |
| **Assimp** | 3D model loading (40+ formats: OBJ, FBX, GLTF, STL, etc.) |
| **Boost.Graph** | Graph algorithms for mesh topology (Dijkstra shortest paths) |
| **HighFive** | HDF5 C++ wrapper for SNIRF file parsing |
| **ImGui + ImPlot** | Immediate-mode GUI and plotting |
| **GLM** | Mathematics library for graphics (vectors, matrices, quaternions) |
| **GLFW + GLEW** | OpenGL context management and extensions |
| **spdlog** | Fast C++ logging library |
| **Eigen** | Linear algebra operations |

Full dependency list in [CMakeLists.txt](CMakeLists.txt).

---

## Building from Source

### Prerequisites

Install required dependencies:

- **Boost** (1.70+): Graph algorithms, filesystem
- **HDF5**: Required for SNIRF file support
- **CMake** (3.15+): Build system
- **C++20 Compiler**: MSVC 2019+, GCC 11+, or Clang 12+

### Build Instructions

#### Windows (Visual Studio)

```bash
# Clone with submodules
git clone --recursive https://github.com/[username]/NIRSViz.git
cd NIRSViz

# Configure
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 ..

# Build
cmake --build . --config Release

# Run
./Release/NIRSViz.exe
```

#### Linux

```bash
# Clone with submodules
git clone --recursive https://github.com/[username]/NIRSViz.git
cd NIRSViz

# Install dependencies (Ubuntu/Debian)
sudo apt-get install libboost-all-dev libhdf5-dev cmake build-essential

# Configure and build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Run
./NIRSViz
```

For detailed build instructions and troubleshooting, see [docs/INSTALLATION.md](docs/INSTALLATION.md).

---

## Documentation

- **[Installation Guide](docs/INSTALLATION.md)**: Detailed setup instructions
- **[User Guide](docs/USER_GUIDE.md)**: Complete usage documentation
- **[Architecture Guide](docs/ARCHITECTURE.md)**: System design and component overview
- **[Benchmarks](docs/BENCHMARKS.md)**: Performance analysis and validation
- **[API Reference](docs/API.md)**: Developer documentation
- **[Contributing Guide](CONTRIBUTING.md)**: How to contribute to NIRSViz

---

## Performance

NIRSViz demonstrates significant performance improvements through advanced spatial indexing:

| Operation | Brute Force | Optimized | Speedup |
|-----------|-------------|-----------|---------|
| Influence Map Computation | 1,144 ms | 17 ms | **65x faster** |
| Ray-Cortex Intersection | O(n) | O(log n) | Logarithmic |
| Vertex Radius Search | O(n) | O(log n + k) | Logarithmic |

*Benchmarks on cortex mesh with ~140,000 vertices. See [docs/BENCHMARKS.md](docs/BENCHMARKS.md) for details.*

---

## Citation

If you use NIRSViz in your research, please cite:

```bibtex
@software{nirsviz2024,
  author = {[Your Name]},
  title = {NIRSViz: Advanced 3D Visualization Platform for fNIRS Data},
  year = {2024},
  url = {https://github.com/adamaske/NIRSViz},
  version = {1.0.0}
}
```

*Academic paper in preparation. Citation will be updated upon publication.*

---

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for:

- Code style guidelines
- Development workflow
- How to submit pull requests
- Testing requirements

---

## License

NIRSViz is licensed under the [MIT License](LICENSE).

Copyright (c) 2024 NIRSViz Contributors

---

## Acknowledgments

### Libraries and Tools

NIRSViz builds upon excellent open-source projects:

- **BVH** by [madmann91](https://github.com/madmann91/bvh) - High-performance BVH library
- **nanoflann** by [Jose Luis Blanco](https://github.com/jlblancoc/nanoflann) - Fast KD-tree implementation
- **Assimp** - Open Asset Import Library
- **Dear ImGui** by Omar Cornut - Immediate-mode GUI
- **ImPlot** - Plotting extension for ImGui
- **Boost C++ Libraries** - Graph algorithms and utilities
- **HighFive** - Modern C++ HDF5 wrapper
- **GLM** - OpenGL Mathematics
- **spdlog** - Fast C++ logging
- **GLFW** - OpenGL context and window management

### fNIRS Community

Special thanks to the fNIRS research community for standardizing the SNIRF data format and advancing non-invasive neuroimaging.

---

## Contact

- **Issues**: [GitHub Issues](https://github.com/[username]/NIRSViz/issues)
- **Discussions**: [GitHub Discussions](https://github.com/[username]/NIRSViz/discussions)
- **Email**: [your-email@institution.edu]

---

## Roadmap

- [ ] GPU-accelerated rendering pipeline
- [ ] Extended SNIRF support (aux data, multi-run experiments)
- [ ] Integration with statistical analysis tools (R, Python)
- [ ] Cloud-based collaborative analysis
- [ ] VR/AR support for immersive visualization
- [ ] Automated probe placement optimization
- [ ] Machine learning-based artifact detection

---

*NIRSViz - Advancing fNIRS visualization through cutting-edge computer graphics and spatial algorithms.*
