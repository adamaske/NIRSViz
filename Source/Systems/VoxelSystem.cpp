#include "pch.h"
#include "Systems/VoxelSystem.h"

#include "Core/AssetRegistry.h"

#include <imgui.h>
#include <random>

void VoxelSystem::OnAttach()
{
    // Setup Voxel Viewport with Free Roam camera
    Viewport3D::Config config;
    config.type = ViewportType::VoxelViewport;
    config.windowTitle = "Voxel Viewport - Brain Sensitivity";
    config.defaultSize = { 1280, 900 };
    config.initialCameraMode = CameraMode::ROAM;  // Free roam camera
    config.initialPosition = { -20.0f, 10.0f, 25.0f };
    config.initialPitch = -15.0f;
    config.initialYaw = 40.0f;

    voxel_viewport_ = CreateScope<Viewport3D>(config);

    SetupRendering();

    // Initialize with default grid configuration
    VoxelGridConfig default_config;
    default_config.dimensions = glm::ivec3(50, 50, 50);
    default_config.origin = glm::vec3(-10.0f, -10.0f, -10.0f);  // Center the grid
    default_config.voxel_size = 1.0f;
    default_config.min_intensity = 0.0f;
    default_config.max_intensity = 1.0f;

    InitializeVoxelGrid(default_config);
    GenerateTestData();
}

void VoxelSystem::OnDetach()
{
}

void VoxelSystem::OnUpdate(DeltaTime dt)
{
    voxel_viewport_->OnUpdate(dt);

    if (rendering_enabled_) {
        // Update animation time for dynamic test data
        animation_time_ += dt;

        RenderVoxels();
    }
}

void VoxelSystem::OnGUIRender()
{
    voxel_viewport_->RenderViewportWindow();

    ImGui::Begin("Voxel System Settings");

    ImGui::SeparatorText("Rendering");
    ImGui::Checkbox("Enable Rendering", &rendering_enabled_);
    ImGui::Checkbox("Wireframe Mode", &show_wireframe_);

    ImGui::SeparatorText("Grid Configuration");
    ImGui::Text("Dimensions: %d x %d x %d",
        grid_config_.dimensions.x,
        grid_config_.dimensions.y,
        grid_config_.dimensions.z);
    ImGui::Text("Total Voxels: %d", voxel_instances_.size());

    if (ImGui::SliderFloat("Voxel Size", &grid_config_.voxel_size, 0.1f, 3.0f)) {
        data_dirty_ = true;
    }

    ImGui::SeparatorText("Heat Map Visualization");
    const char* colormap_names[] = { "Viridis", "Jet (Brain)", "Grayscale" };
    ImGui::Combo("Colormap", &selected_colormap_, colormap_names, 3);

    ImGui::SliderFloat("Transparency Threshold", &transparency_threshold_, 0.0f, 0.5f);
    ImGui::SliderFloat("Min Intensity", &grid_config_.min_intensity, 0.0f, 1.0f);
    ImGui::SliderFloat("Max Intensity", &grid_config_.max_intensity, 0.0f, 2.0f);

    ImGui::SeparatorText("Lighting Quality");
    ImGui::SliderFloat("Ambient Light", &ambient_strength_, 0.0f, 1.0f);
    ImGui::SliderFloat("Specular Strength", &specular_strength_, 0.0f, 1.0f);
    ImGui::SliderInt("Shininess", &shininess_, 1, 128);

    if (ImGui::Button("Reset Lighting to Default")) {
        ambient_strength_ = 0.3f;
        specular_strength_ = 0.4f;
        shininess_ = 32;
    }

    ImGui::SeparatorText("Test Data");
    if (ImGui::Button("Regenerate Test Data")) {
        GenerateTestData();
    }

    ImGui::SeparatorText("Viewport Camera");
    voxel_viewport_->RenderCameraSettings(false);

    ImGui::End();
}

void VoxelSystem::OnEvent(Event& event)
{
    voxel_viewport_->OnEvent(event);
}

void VoxelSystem::RenderMenuBar()
{
    if (ImGui::BeginMenu("Voxel")) {
        if (ImGui::MenuItem("Toggle Rendering", nullptr, rendering_enabled_)) {
            rendering_enabled_ = !rendering_enabled_;
        }

        if (ImGui::MenuItem("Regenerate Test Data")) {
            GenerateTestData();
        }

        ImGui::EndMenu();
    }
}

void VoxelSystem::SetupRendering()
{
    // Load voxel shader
    voxel_shader_ = CreateRef<Shader>(
        AssetRegistry::Get("Voxel.vert"),
        AssetRegistry::Get("Voxel.frag")
    );

    CreateUnitCubeMesh();
    CreateInstanceBuffer();
}

void VoxelSystem::CreateUnitCubeMesh()
{
    // Unit cube vertices with positions and normals (0 to 1 in each dimension)
    // Format: x, y, z, nx, ny, nz (position + normal for each vertex)
    float vertices[] = {
        // Front face (Z+)
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,

        // Back face (Z-)
        1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,

        // Left face (X-)
        0.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,

        // Right face (X+)
        1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,

        // Top face (Y+)
        0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,

        // Bottom face (Y-)
        0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f
    };

    // Cube indices (2 triangles per face)
    uint32_t indices[] = {
        // Front
        0, 1, 2,  2, 3, 0,
        // Back
        4, 5, 6,  6, 7, 4,
        // Left
        8, 9, 10,  10, 11, 8,
        // Right
        12, 13, 14,  14, 15, 12,
        // Top
        16, 17, 18,  18, 19, 16,
        // Bottom
        20, 21, 22,  22, 23, 20
    };

    cube_vao_ = CreateRef<VertexArray>();
    cube_vbo_ = CreateRef<VertexBuffer>(vertices, sizeof(vertices));

    BufferLayout layout = {
        { ShaderDataType::Float3, "a_Position" },
        { ShaderDataType::Float3, "a_Normal" }
    };
    cube_vbo_->SetLayout(layout);

    cube_vao_->AddVertexBuffer(cube_vbo_);

    cube_ibo_ = CreateRef<IndexBuffer>(indices, sizeof(indices) / sizeof(uint32_t));
    cube_vao_->SetIndexBuffer(cube_ibo_);
}

void VoxelSystem::CreateInstanceBuffer()
{
    // Create instance buffer (will be updated with actual voxel data later)
    // Allocate initial size for the buffer
    instance_vbo_ = CreateRef<VertexBuffer>(1024);  // Initial allocation

    BufferLayout instance_layout = {
        { ShaderDataType::Float3, "a_InstancePosition" },
        { ShaderDataType::Float,  "a_InstanceIntensity" }
    };
    instance_vbo_->SetLayout(instance_layout);

    cube_vao_->AddVertexBuffer(instance_vbo_, true);  // Mark as instanced
}

void VoxelSystem::InitializeVoxelGrid(const VoxelGridConfig& config)
{
    grid_config_ = config;
    voxel_instances_.clear();

    int total_voxels = config.dimensions.x * config.dimensions.y * config.dimensions.z;
    voxel_instances_.reserve(total_voxels);

    // Create voxel instances
    for (int z = 0; z < config.dimensions.z; ++z) {
        for (int y = 0; y < config.dimensions.y; ++y) {
            for (int x = 0; x < config.dimensions.x; ++x) {
                VoxelInstance voxel;
                voxel.position = config.origin + glm::vec3(x, y, z) * config.voxel_size;
                voxel.intensity = 0.0f;  // Default intensity
                voxel_instances_.push_back(voxel);
            }
        }
    }

    data_dirty_ = true;
}

void VoxelSystem::UpdateVoxelData(const std::vector<VoxelInstance>& voxels)
{
    if (voxels.size() != voxel_instances_.size()) {
        // Size mismatch - reinitialize
        voxel_instances_ = voxels;
    } else {
        // Update existing data
        voxel_instances_ = voxels;
    }
    data_dirty_ = true;
}

void VoxelSystem::SetIntensity(int x, int y, int z, float intensity)
{
    if (x < 0 || x >= grid_config_.dimensions.x ||
        y < 0 || y >= grid_config_.dimensions.y ||
        z < 0 || z >= grid_config_.dimensions.z) {
        return;  // Out of bounds
    }

    int index = x + y * grid_config_.dimensions.x +
                z * grid_config_.dimensions.x * grid_config_.dimensions.y;

    if (index >= 0 && index < voxel_instances_.size()) {
        voxel_instances_[index].intensity = intensity;
        data_dirty_ = true;
    }
}

float VoxelSystem::GetIntensity(int x, int y, int z) const
{
    if (x < 0 || x >= grid_config_.dimensions.x ||
        y < 0 || y >= grid_config_.dimensions.y ||
        z < 0 || z >= grid_config_.dimensions.z) {
        return 0.0f;  // Out of bounds
    }

    int index = x + y * grid_config_.dimensions.x +
                z * grid_config_.dimensions.x * grid_config_.dimensions.y;

    if (index >= 0 && index < voxel_instances_.size()) {
        return voxel_instances_[index].intensity;
    }
    return 0.0f;
}

void VoxelSystem::GenerateTestData()
{
    // Generate interesting test data - 3D gaussian blob with some noise
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> noise_dist(0.0f, 0.1f);

    glm::vec3 center = glm::vec3(grid_config_.dimensions) * 0.5f;
    float max_dist = glm::length(glm::vec3(grid_config_.dimensions) * 0.5f);

    for (int z = 0; z < grid_config_.dimensions.z; ++z) {
        for (int y = 0; y < grid_config_.dimensions.y; ++y) {
            for (int x = 0; x < grid_config_.dimensions.x; ++x) {
                glm::vec3 pos(x, y, z);
                float dist = glm::length(pos - center);
                float normalized_dist = dist / max_dist;

                // Gaussian falloff with noise
                float intensity = std::exp(-normalized_dist * normalized_dist * 4.0f);
                intensity += noise_dist(gen);
                intensity = glm::clamp(intensity, 0.0f, 1.0f);

                SetIntensity(x, y, z, intensity);
            }
        }
    }

    data_dirty_ = true;
}

void VoxelSystem::UpdateInstanceBuffer()
{
    if (!data_dirty_) return;

    // Upload voxel instance data to GPU
    instance_vbo_->SetData(voxel_instances_.data(),
                           voxel_instances_.size() * sizeof(VoxelInstance));

    data_dirty_ = false;
}

void VoxelSystem::RenderVoxels()
{
    if (voxel_instances_.empty()) return;

    // Update GPU buffer if data has changed
    UpdateInstanceBuffer();

    // Build render command
    RenderCommand cmd;
    cmd.ShaderPtr = voxel_shader_.get();
    cmd.VAOPtr = cube_vao_.get();
    cmd.target_viewport = ViewportType::VoxelViewport;
    cmd.Transform = glm::mat4(1.0f);  // Identity - transformations handled per-instance
    cmd.Mode = DRAW_ELEMENTS_INSTANCED;
    cmd.InstanceCount = static_cast<uint32_t>(voxel_instances_.size());

    // Set uniforms
    UniformData voxel_size_uniform;
    voxel_size_uniform.Type = UniformDataType::FLOAT1;
    voxel_size_uniform.Name = "u_VoxelSize";
    voxel_size_uniform.Data.f1 = grid_config_.voxel_size;

    UniformData min_intensity_uniform;
    min_intensity_uniform.Type = UniformDataType::FLOAT1;
    min_intensity_uniform.Name = "u_MinIntensity";
    min_intensity_uniform.Data.f1 = grid_config_.min_intensity;

    UniformData max_intensity_uniform;
    max_intensity_uniform.Type = UniformDataType::FLOAT1;
    max_intensity_uniform.Name = "u_MaxIntensity";
    max_intensity_uniform.Data.f1 = grid_config_.max_intensity;

    UniformData transparency_threshold_uniform;
    transparency_threshold_uniform.Type = UniformDataType::FLOAT1;
    transparency_threshold_uniform.Name = "u_TransparencyThreshold";
    transparency_threshold_uniform.Data.f1 = transparency_threshold_;

    UniformData colormap_uniform;
    colormap_uniform.Type = UniformDataType::INT1;
    colormap_uniform.Name = "u_Colormap";
    colormap_uniform.Data.i1 = selected_colormap_;

    UniformData ambient_uniform;
    ambient_uniform.Type = UniformDataType::FLOAT1;
    ambient_uniform.Name = "u_AmbientStrength";
    ambient_uniform.Data.f1 = ambient_strength_;

    UniformData specular_uniform;
    specular_uniform.Type = UniformDataType::FLOAT1;
    specular_uniform.Name = "u_SpecularStrength";
    specular_uniform.Data.f1 = specular_strength_;

    UniformData shininess_uniform;
    shininess_uniform.Type = UniformDataType::INT1;
    shininess_uniform.Name = "u_Shininess";
    shininess_uniform.Data.i1 = shininess_;

    cmd.UniformCommands = {
        voxel_size_uniform,
        min_intensity_uniform,
        max_intensity_uniform,
        transparency_threshold_uniform,
        colormap_uniform,
        ambient_uniform,
        specular_uniform,
        shininess_uniform
    };

    // Add API calls for enabling/disabling features
    if (show_wireframe_) {
        cmd.APICalls.push_back({ []() { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); } });
    }

    // Enable blending for transparency
    cmd.APICalls.push_back({ []() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } });

    Renderer::Submit(cmd);

    // Reset wireframe mode
    if (show_wireframe_) {
        RendererAPICall reset_wireframe;
        reset_wireframe.call = []() { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); };
        // We'd need to submit another command or handle this differently
        // For now, this is a limitation - wireframe persists
    }
}
