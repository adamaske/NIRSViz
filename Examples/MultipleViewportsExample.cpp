// Example: Creating multiple 3D viewports easily

#include "pch.h"
#include "Renderer/Viewport/Viewport3D.h"

class MyApplication {
public:
    void SetupViewports() {

        // Example 1: Main anatomy viewport
        {
            Viewport3D::Config config;
            config.type = ViewportType::MainViewport;
            config.windowTitle = "Main Anatomy View";
            config.defaultSize = { 800, 600 };
            config.initialCameraMode = CameraMode::ROAM;
            config.initialPosition = { -14.0f, 5.0f, -15.0f };
            config.initialYaw = 50.0f;
            config.initialPitch = -15.0f;

            main_viewport_ = CreateScope<Viewport3D>(config);
        }

        // Example 2: Probe editor viewport
        {
            Viewport3D::Config config;
            config.type = ViewportType::ProbeEditor;
            config.windowTitle = "Probe Editor";
            config.defaultSize = { 400, 400 };
            config.initialCameraMode = CameraMode::ORBIT;  // Start in orbit mode
            config.initialPosition = { 0.0f, 10.0f, 0.0f };
            config.initialOrbitRadius = 15.0f;

            probe_viewport_ = CreateScope<Viewport3D>(config);
        }

        // Example 3: Top-down view viewport
        {
            Viewport3D::Config config;
            config.type = ViewportType::AtlasViewport;
            config.windowTitle = "Top-Down View";
            config.defaultSize = { 400, 400 };
            config.initialCameraMode = CameraMode::ORBIT;
            config.initialPosition = { 0.0f, 20.0f, 0.0f };
            config.initialPitch = -90.0f;  // Look straight down
            config.initialOrbitRadius = 20.0f;

            topdown_viewport_ = CreateScope<Viewport3D>(config);
        }
    }

    void OnUpdate(float dt) {
        // Just forward the update calls
        main_viewport_->OnUpdate(dt);
        probe_viewport_->OnUpdate(dt);
        topdown_viewport_->OnUpdate(dt);
    }

    void OnEvent(Event& event) {
        // Forward events to all viewports
        main_viewport_->OnEvent(event);
        probe_viewport_->OnEvent(event);
        topdown_viewport_->OnEvent(event);
    }

    void OnImGuiRender() {
        // Render all viewports
        main_viewport_->RenderViewportWindow();
        probe_viewport_->RenderViewportWindow();
        topdown_viewport_->RenderViewportWindow();

        // Optional: Combined camera settings window
        if (ImGui::Begin("All Camera Settings")) {
            if (ImGui::CollapsingHeader("Main Viewport Camera")) {
                main_viewport_->RenderCameraSettings(false);
            }
            if (ImGui::CollapsingHeader("Probe Editor Camera")) {
                probe_viewport_->RenderCameraSettings(false);
            }
            if (ImGui::CollapsingHeader("Top-Down Camera")) {
                topdown_viewport_->RenderCameraSettings(false);
            }
            ImGui::End();
        }
    }

private:
    Scope<Viewport3D> main_viewport_;
    Scope<Viewport3D> probe_viewport_;
    Scope<Viewport3D> topdown_viewport_;
};


// Another example: Simple standalone viewport usage
void CreateQuickViewport() {
    Viewport3D::Config config;
    config.type = ViewportType::MainViewport;
    config.windowTitle = "My Viewport";

    auto viewport = CreateScope<Viewport3D>(config);

    // That's it! The viewport is ready to use
    // Just call:
    // - viewport->OnUpdate(dt) in your update loop
    // - viewport->OnEvent(event) for events
    // - viewport->RenderViewportWindow() to display it
}
