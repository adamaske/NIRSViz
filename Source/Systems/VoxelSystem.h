#pragma once

#include "Systems/System.h"
#include "Renderer/Viewport/Viewport3D.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Buffer/VertexArray.h"
#include "Renderer/Buffer/VertexBuffer.h"
#include "Renderer/Buffer/IndexBuffer.h"

#include <glm/glm.hpp>
#include <vector>

// Voxel data structure - position and intensity for heat map visualization
struct VoxelInstance {
    glm::vec3 position;
    float intensity;  // 0.0 to 1.0, maps to heat map color
};

// Configuration for voxel grid
struct VoxelGridConfig {
    glm::ivec3 dimensions = glm::ivec3(20, 20, 20);  // Grid dimensions (x, y, z)
    glm::vec3 origin = glm::vec3(0.0f);              // Grid origin in world space
    float voxel_size = 1.0f;                          // Size of each voxel cube
    float min_intensity = 0.0f;                       // Minimum intensity value
    float max_intensity = 1.0f;                       // Maximum intensity value
};

class VoxelSystem : public System {
public:
    VoxelSystem() = default;
    ~VoxelSystem() = default;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(DeltaTime dt) override;
    void OnGUIRender() override;
    void OnEvent(Event& event) override;
    void RenderMenuBar() override;

    // Voxel grid management
    void InitializeVoxelGrid(const VoxelGridConfig& config);
    void UpdateVoxelData(const std::vector<VoxelInstance>& voxels);
    void SetIntensity(int x, int y, int z, float intensity);
    float GetIntensity(int x, int y, int z) const;

    // Rendering control
    void SetRenderingEnabled(bool enabled) { rendering_enabled_ = enabled; }
    bool IsRenderingEnabled() const { return rendering_enabled_; }

    // Getters
    const VoxelGridConfig& GetGridConfig() const { return grid_config_; }
    VoxelGridConfig& GetGridConfigMutable() { return grid_config_; }

private:
    // Initialization helpers
    void SetupRendering();
    void CreateUnitCubeMesh();
    void CreateInstanceBuffer();
    void GenerateTestData();

    // Rendering
    void RenderVoxels();
    void UpdateInstanceBuffer();

    // Viewport and rendering resources
    Scope<Viewport3D> voxel_viewport_;
    Ref<Shader> voxel_shader_;

    // Unit cube mesh (rendered as instanced)
    Ref<VertexArray> cube_vao_;
    Ref<VertexBuffer> cube_vbo_;
    Ref<IndexBuffer> cube_ibo_;

    // Instance data buffer (per-voxel data)
    Ref<VertexBuffer> instance_vbo_;

    // Voxel grid data
    VoxelGridConfig grid_config_;
    std::vector<VoxelInstance> voxel_instances_;
    bool data_dirty_ = false;  // Flag to update GPU buffers

    // Rendering state
    bool rendering_enabled_ = true;
    float animation_time_ = 0.0f;  // For animated test data

    // UI settings
    bool show_wireframe_ = false;
    float transparency_threshold_ = 0.01f;  // Voxels below this intensity become more transparent
    int selected_colormap_ = 1;  // 0 = Viridis, 1 = Jet (default), 2 = Grayscale

    // Lighting settings
    float ambient_strength_ = 0.3f;
    float specular_strength_ = 0.4f;
    int shininess_ = 32;
};
