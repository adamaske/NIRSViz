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

### Core Pattern: Systems + Services over a GLFW/OpenGL backend

```
┌─────────────────────────────────────┐
│         ImGui Interface             │
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

### Memory Management Types

```cpp
Ref<T>   = std::shared_ptr<T>   // CreateRef<T>(...)
Scope<T> = std::unique_ptr<T>   // CreateScope<T>(...)
DeltaTime = double
```

### System Architecture

All pluggable behavior lives in Systems. Add `SystemManager.h` includes the full list.

```cpp
class System {
    virtual void OnAttach();    // called once when system is registered
    virtual void OnDetach();    // called on teardown
    virtual void OnUpdate(DeltaTime dt);
    virtual void OnGUIRender(); // ImGui panels
    virtual void OnEvent(Event&);
    virtual void RenderMenuBar();
};
```

Systems registered in `Application::Application()` via:
```cpp
system_manager_.AddSystem<MySystem>(/*ctor args*/);
```

Lookup from anywhere with access to Application:
```cpp
auto sys = Application::Get().GetSystem<ProbeSystem>();
```

Current systems: `FileSystem`, `AnatomySystem`, `ProbeSystem`, `ProjectionSystem`, `PlottingSystem`, `BiosignalPlottingSystem`, `ChannelSelectorSystem`, `ControlPanelSystem`, `VoxelSystem`, `MRISystem`, `ImGuiSystem`.

### Service Architecture

Services are cross-cutting singletons. Two ownership patterns exist:

**Application-owned** (via `Scope<>` in `Application`): `SNIRFService`, `AnatomyService`, `SessionService` — accessed through `Application::Get().GetSessionService()` etc.

**Self-owned singletons** (static instance): `ConfigStore`, `ProjectService`, `NotificationService` — accessed via `::Get()`:
```cpp
ProjectService::Get().SaveProject();
NotificationService::Get().Notify("Loaded.", NotificationLevel::Info);
ConfigStore::Get<float>("ProbeSystem.spread_factor", 0.11f);
```

### Settings: ConfigStore vs ProjectService

- **ConfigStore** (`config.ini`) — per-machine transient settings (window layout, theme, UI state). Use `CONFIG_SAVE`/`CONFIG_LOAD` macros in system `OnAttach`/`OnDetach`.
- **ProjectService** (`.nirsv` YAML) — reproducible scientific session state (data paths, probe transforms, projection settings, channel selections).

```cpp
// In OnDetach / SaveSettings:
CONFIG_SAVE("ProbeSystem", spread_factor_);
// In OnAttach / LoadSettings:
CONFIG_LOAD("ProbeSystem", spread_factor_);
```

### Event System

Events flow from `Window` → `Application::OnEvent` → dispatched to all systems via `System::OnEvent`. Use `EventDispatcher` to handle specific types:

```cpp
void MySystem::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(OnKeyPressed));
}
```

`EventBus.h` is present but currently commented out — do not use it.

### Null-Safety Rules

Always guard anatomy and SNIRF access before use:

```cpp
// Anatomy
IAnatomyProvider& anatomy = ...;
if (!anatomy.HasCortex()) return;
auto& cortex = anatomy.GetCortexMutable();

// SNIRF
SNIRFService& snirf_svc = ...;
if (!snirf_svc.IsLoaded()) return;
auto& snirf = snirf_svc.GetSNIRF();
```

### Logging

Use the `NVIZ_*` macros (backed by spdlog). Output goes to console + the in-app ImGui Log panel via `ImGuiLogSink`.

```cpp
NVIZ_INFO("Loaded {} channels", count);
NVIZ_WARN("No cortex loaded");
NVIZ_ERROR("Failed to open: {}", path.string());
```

### Key Domain Concepts

**fNIRS Data Flow:**
1. Load SNIRF file → `SNIRFLoader` (thin adapter over `snirf-cpp` submodule) → `SNIRFService`
2. Load MRI/anatomy → `AnatomySystem` → `AnatomyService`
3. Match landmarks for coordinate transforms
4. Project probe channels onto cortex surface (`ProjectionSystem`)
5. Render with OpenGL
6. Display biosignals in time-series plots (`BiosignalPlottingSystem`)

**Core Types:**
- `Optode` — light source or detector with 2D/3D positions
- `Channel` — source-detector pair with HbO/HbR/HbT time-series data
- `Probe` — collection of channels and optodes
- `Wavelength` — `HBR` (deoxygenated), `HBO` (oxygenated), `HBT` (total)

**Anatomy Types** (`Source/NIRS/Anatomy/`):
- `NIRS::Head` — scalp surface mesh
- `NIRS::Cortex` — cortex surface mesh
- Both accessed through `IAnatomyProvider` interface

### Viewport System

Multiple viewport types managed by `ViewportManager`. Viewports are either `Viewport2D` or `Viewport3D` subtypes.

Current viewports: `MainViewport`, `AnatomyViewport`, `AtlasViewport`, `ProbeEditor`, `ChannelSelector`, `VoxelViewport`, `MRIViewport`.

### SNIRF Loading (snirf-cpp submodule)

`vendor/snirf-cpp` is a static lib (`snirf` CMake target). `SNIRFLoader.cpp` is a thin adapter:

```cpp
// Public API — unchanged from caller perspective:
NIRS::LoadSNIRF(path, out_snirf, errors);

// Internally calls:
SNIRFCPP::SNIRFFactory::CreateSNIRF(out, filepath, type, errors);
// then maps SNIRFCPP:: types → NIRSViz GLM/Eigen types
```

### Key Dependencies

| Library | Purpose |
|---------|---------|
| ImGui/ImPlot | Immediate-mode GUI and plotting |
| OpenGL/GLFW | Graphics rendering |
| ITK | Medical image processing (NIFTI support) |
| Assimp | 3D model loading |
| Boost.Graph | Graph algorithms |
| HDF5/HighFive | Scientific data I/O (via snirf-cpp) |
| spdlog/fmt | Logging |
| GLM | Math (vec2/3/4, mat4) |
| nanoflann | KD-tree for nearest-neighbour queries |
| yaml-cpp | Project file (.nirsv) serialization |

### Coordinate Systems

- Local mesh space (with `Transform`)
- World space (absolute positioning)
- 2D head coordinates (probe layouts)
- 3D stereotactic coordinates (anatomy)

## Code Conventions

- Headers: `.h`, Implementation: `.cpp`
- Classes/types: `CamelCase`
- Variables/members: `snake_case_` (trailing underscore for private members)
- Files match class names
- GLM types used extensively: `glm::vec2/3/4`, `glm::mat4`
- `ConfigValue` uses `std::variant` (see `ConfigStore::Value`)
