#pragma once

#include "Systems/System.h"

#include "NIRS/Anatomy/Cortex.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Shader.h"

#include "MRI/MRISliceViewer.h"

class AnatomyService;
namespace NVMRI {
	class MRIImage;
	class MRIVolumetricImage;
}

class MRISystem : public System {

public:
	MRISystem() = default;
	~MRISystem() {};

	void OnAttach() override;
	void OnUpdate(DeltaTime dt) override;
	void OnGUIRender() override;
	void RenderMenuBar() override;

	bool LoadMRI(const std::filesystem::path& path);

private:
	// Shaders
	Ref<Shader> phong_shader_;
	Ref<Shader> flat_shader_;
	Ref<Shader> slice_plane_shader_;

	// MRI data
	Ref<NVMRI::MRIImage> mri_image_;
	Ref<NVMRI::MRIVolumetricImage> volumetric_image_;

	// GUI panels
	void RenderMRIMetadataPanel();

	// Slice Viewer
	Scope<NVMRI::MRISliceViewer> slice_viewer_;
};