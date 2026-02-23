# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

NIRSViz is a C++ medical visualization application for fNIRS (functional Near-Infrared Spectroscopy) data combined with MRI imaging. It visualizes brain activity measurements (hemoglobin concentration changes) mapped onto 3D brain anatomy models.

## Build Commands

```bash
# Configure (from project root)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Debug build
cmake --build build --config Debug

# Run executable
./build/Release/NIRSViz.exe   # Windows Release
./build/Debug/NIRSViz.exe     # Windows Debug
```

**Prerequisites:**
- Visual Studio 2022 (MSVC)
- CMake 3.15+
- Qt6 (Widgets, OpenGL, OpenGLWidgets, GUI)
- Boost 1.90.0 at `C:\sdk\boost_1_90_0`
- HDF5 2.0.0 at `C:/Program Files/HDF_Group/HDF5/2.0.0`
- vcpkg (for ITK)

## Architecture

### Core Pattern: Entity-Component-System with Services

```
┌─────────────────────────────────────┐
│     Qt6 GUI / ImGui Interface       │
├─────────────────────────────────────┤
│  Systems (pluggable modules)        │
├─────────────────────────────────────┤
│  Services (cross-cutting concerns)  │
├─────────────────────────────────────┤
│  Renderer (OpenGL/GLFW backend)     │
├─────────────────────────────────────┤
│  Core (Events, Window, Input, Logs) │
└─────────────────────────────────────┘
```

### Entry Point & Lifecycle

`Source/main.cpp` → `Application` singleton → `app.Run()` main loop

Application initializes: logging → config → GLFW window → OpenGL → attach systems → initialize services

### Key Directories

- `Source/` - Application code
- `Source/Systems/` - Pluggable system modules (11 systems: FileSystem, ProjectionSystem, ProbeSystem, PlottingSystem, AnatomySystem, etc.)
- `Source/Services/` - Singletons (SNIRFService, AnatomyService, SessionService, ConfigService, FileDialogService)
- `Source/Renderer/` - OpenGL rendering (Camera, Mesh, Buffer, Shader, Viewport)
- `Source/NIRS/` - fNIRS domain types (Probe, Channel, Optode, Biosignals)
- `Source/MRI/` - Medical imaging (MRIImage, MRIVolumetricImage, MRISliceViewer)
- `Source/Core/` - Foundation (Application, Window, Input, Log, ConfigStore)
- `Source/Events/` - Event system (keyboard, mouse, application events)
- `Source/GUI/` - ImGui panels and UI components
- `vendor/` - Third-party libraries (git submodules)

### System Architecture

Systems inherit from `System` base class with lifecycle methods:
```cpp
class System {
    virtual void OnAttach();
    virtual void OnDetach();
    virtual void OnUpdate(DeltaTime dt);
    virtual void OnGUIRender();
    virtual void OnEvent(Event&);
    virtual void RenderMenuBar();
};
```

SystemManager provides template-based lookup:
```cpp
template<typename T> T* AddSystem(args...);
template<typename T> Ref<T> GetSystem();
```

### Memory Management Types

```cpp
Ref<T>   = std::shared_ptr<T>   // Shared ownership
Scope<T> = std::unique_ptr<T>   // Exclusive ownership
```

### Key Domain Concepts

**fNIRS Data Flow:**
1. Load SNIRF file → SNIRFLoader
2. Load MRI/anatomy → AnatomySystem
3. Match landmarks for coordinate transforms
4. Project probe channels onto cortex surface
5. Render with OpenGL
6. Display biosignals in time-series plots

**Core Types:**
- `Optode` - Light source or detector with 2D/3D positions
- `Channel` - Source-detector pair with HbO/HbR/HbT data
- `Probe` - Collection of channels and optodes
- `Wavelength` - HBR (deoxygenated), HBO (oxygenated), HBT (total)

### Viewport System

Multiple viewport types managed by ViewportManager:
- MainViewport, AnatomyViewport, AtlasViewport, ProbeEditor, ChannelSelector, VoxelViewport, MRIViewport

### Key Dependencies

| Library | Purpose |
|---------|---------|
| Qt6 | UI framework |
| ImGui/ImPlot | Immediate-mode GUI and plotting |
| OpenGL/GLFW | Graphics rendering |
| ITK | Medical image processing (NIFTI support) |
| Assimp | 3D model loading (40+ formats) |
| Boost.Graph | Graph algorithms |
| HDF5/HighFive | Scientific data I/O |
| spdlog/fmt | Logging |
| GLM | Math (vectors, matrices) |
| nanoflann | KD-tree for nearest neighbor |
| BVH | Spatial acceleration for ray casting |

### Coordinate Systems

- Local mesh space (with Transform)
- World space (absolute positioning)
- 2D head coordinates (probe layouts)
- 3D stereotactic coordinates (anatomy)

## Code Conventions

- Headers: `.h`, Implementation: `.cpp`
- Classes/types: CamelCase
- Variables: snake_case
- Files match class names
- Uses GLM types extensively (glm::vec2/3/4, glm::mat4)
- Heavy use of std::variant for ConfigValue
