# NIRSViz

A real-time visualization tool for functional Near-Infrared Spectroscopy (fNIRS) data combined with structural MRI. NIRSViz maps hemoglobin concentration changes from fNIRS recordings onto 3D brain anatomy models, enabling spatial analysis of cortical activation patterns.

## Features

- Load and parse SNIRF files (HDF5-based fNIRS standard)
- Render head and cortex surface meshes from OBJ files
- Project fNIRS channel signals onto the cortex surface with configurable influence radius
- Time-series biosignal plotting (HbO, HbR, HbT) via ImPlot
- MRI slice viewer with volumetric image support (NIFTI via ITK)
- Probe editor — visualize and adjust optode/channel layouts in 2D and 3D
- Channel selector for isolating specific source-detector pairs
- Landmark-based coordinate system alignment between probe space and anatomy space
- Project file format (`.nirsv`) for saving and reloading session state
- Per-machine settings persisted across sessions (window layout, rendering options)

## Prerequisites

| Requirement | Version |
|-------------|---------|
| Visual Studio | 2022 (MSVC) |
| CMake | 3.15+ |
| Qt6 | Widgets, OpenGL, OpenGLWidgets, GUI |
| Boost | 1.90.0 at `C:\sdk\boost_1_90_0` |
| HDF5 | 2.0.0 at `C:/Program Files/HDF_Group/HDF5/2.0.0` |
| vcpkg | required for ITK |

All other dependencies (ImGui, ImPlot, GLFW, GLM, Assimp, spdlog, yaml-cpp, etc.) are included as git submodules under `vendor/`.

## Build

```bash
# Clone with submodules
git clone --recurse-submodules <repo-url>
cd NIRSViz

# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run
./build/Release/NIRSViz.exe
```

For a debug build, replace `Release` with `Debug` throughout.

## Getting Started

1. **Load anatomy** — use the menu to load a head mesh (OBJ) and a cortex mesh (OBJ).
2. **Load fNIRS data** — open a `.snirf` file via File > Open or the SNIRF panel.
3. **Align coordinate systems** — place landmarks to register probe space to anatomy space.
4. **Project to cortex** — enable projection in the Projection panel to map channel activations onto the cortex surface.
5. **Inspect signals** — use the Biosignal Plotting panel to view time-series data; scrub the timeline to animate the cortex projection.
6. **Save your session** — File > Save (Ctrl+S) writes a `.nirsv` project file capturing all paths and settings.

Sample data files are available in `Assets/NIRS/` and sample meshes in `Assets/Models/`.

## Project File Format

NIRSViz project files (`.nirsv`) are YAML documents that store the reproducible scientific state of a session — data file paths, probe transforms, projection settings, and channel selections. Per-machine preferences (window layout, theme) are stored separately in `config.ini` and are never written to the project file.

## Architecture

NIRSViz uses a System/Service architecture over an OpenGL/GLFW backend with an ImGui UI layer.

- **Systems** (`Source/Systems/`) — pluggable modules with `OnAttach`/`OnUpdate`/`OnGUIRender`/`OnEvent` lifecycle methods. Each system owns a functional domain (anatomy rendering, projection, plotting, etc.).
- **Services** (`Source/Services/`) — cross-cutting singletons accessed globally (`SNIRFService`, `AnatomyService`, `ProjectService`, `NotificationService`, `ConfigStore`).
- **Renderer** (`Source/Renderer/`) — OpenGL abstractions: meshes, shaders, buffers, cameras, and viewports.
- **NIRS domain** (`Source/NIRS/`) — core types: `Probe`, `Channel`, `Optode`, `Head`, `Cortex`, coordinate systems, and landmark calculators.
- **MRI** (`Source/MRI/`) — volumetric image loading and 2D slice viewer.

SNIRF parsing is handled by the `snirf-cpp` submodule (`vendor/snirf-cpp`), wrapped by a thin adapter in `Source/NIRS/SNIRFLoader.cpp`.

## License

MIT — see [LICENSE.txt](LICENSE.txt).
