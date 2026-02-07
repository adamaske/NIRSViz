#pragma once

#include "Systems/System.h"

#include "MRI/MRIImage.h"
#include "MRI/MRISliceViewer.h"

#include "Renderer/Viewport/Viewport3D.h"

#include "NIRS/Anatomy/Cortex.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Shader.h"

class MRISystem : public System {

public:

	void OnAttach() override;
	void OnUpdate(DeltaTime dt) override;
	void OnGUIRender() override;
	void RenderMenuBar() override;

	bool LoadMRI(const std::filesystem::path& path);

	// Shaders
	Ref<Shader> phong_shader_;
	Ref<Shader> flat_shader_;
	Ref<Shader> slice_plane_shader_;

	// MRI data
	Ref<NVMRI::MRIImage> mri_image_;
	Scope<Viewport3D> mri_viewport_;

	// Cortex rendering
	Scope<NIRS::Cortex> cortex_;
	void SetupCortexRendering();
	void UpdateCortexRenderCommand();
	RenderCommand cortex_render_command_;

	// GUI panels
	void RenderMRIMetadataPanel();

	// Slice Viewer
	Scope<NVMRI::MRISliceViewer> slice_viewer_;
	
};