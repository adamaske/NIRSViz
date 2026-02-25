#pragma once
#include <vector>
#include <array>
#include "Core/Base.h"
#include "MRI/MRIImage.h"

#include "Renderer/Buffer/VertexArray.h"
#include "Renderer/Buffer/IndexBuffer.h"
#include "Renderer/Buffer/VertexBuffer.h"

#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderable/Texture.h"

#include "App/Data/Transform.h"

enum class SlicePlane {
	Axial = 0,    // XY plane, cursor.z varies
	Sagittal = 1, // YZ plane, cursor.x varies
	Coronal = 2   // XZ plane, cursor.y varies
};

struct Slice {
	SlicePlane plane = SlicePlane::Axial;
	Ref<Texture> texture;
	bool needs_update = true;
};

namespace NVMRI {

	class MRISliceViewer {
	public:
		MRISliceViewer() = default;
		~MRISliceViewer() = default;

		void OnAttach(NVMRI::MRIImage* mri_image);
		void OnUpdate(DeltaTime dt);
		void Render(bool standalone);
		void Render2DViewer(bool standalone);

		// Set cursor position in voxel coordinates (0 to dims-1)
		void SetCursorVoxel(const glm::vec3& voxel_pos);
		// Set cursor position as normalized (0 to 1) in each dimension
		void SetCursorNormalized(const glm::vec3& normalized_pos);
		// Get current cursor in voxel coordinates
		glm::vec3 GetCursorVoxel() const { return cursor_voxel_; }
		// Get current cursor as normalized (0 to 1)
		glm::vec3 GetCursorNormalized() const;

		// Get slice index for a given plane based on current cursor
		uint32_t GetSliceIndex(SlicePlane plane) const;
		// Get the dimension that varies for each plane
		static uint32_t GetVaryingDimension(SlicePlane plane);

	private:
		void DrawSlice(Slice& slice);
		void UpdateSliceTexture(int sliceIndex);
		void UpdateAllSliceTextures();
		void DrawCrosshairs();

		glm::mat4 ComputeSliceTransform(SlicePlane plane) const;
		std::vector<uint8_t> ExtractSliceRGBA8(SlicePlane plane, uint32_t sliceIdx);

		// Compute crosshair position on a slice (returns UV coordinates 0-1)
		glm::vec2 GetCrosshairUV(SlicePlane plane) const;

	private:
		// 3D cursor position in voxel coordinates - all slices reference this
		glm::vec3 cursor_voxel_ = glm::vec3(0.0f);

		// Window/level for intensity mapping
		float window_center_ = 0.5f;
		float window_width_ = 1.0f;

		// Crosshair settings
		bool show_crosshairs_ = true;
		glm::vec3 crosshair_color_ = glm::vec3(1.0f, 1.0f, 0.0f); // Yellow

		// Manual alignment controls
		bool use_world_transform_ = false;  // If false, slices align to voxel axes (default: aligned)
		bool box_edge_mode_ = true;        // When true, planes are fixed at the volume boundary edges
		glm::vec3 rotation_offset_deg_ = glm::vec3(0.0f, 0.0f, 0.0f);
		float scale_factor_ = 1.0f;  // Overall scale factor

	public:
		std::array<Slice, 3> slices_;
		MRIImage* mri_image_ = nullptr;

		// Quad geometry for slice rendering
		Ref<VertexArray> plane_vao_;
		Ref<VertexBuffer> plane_vbo_;
		Ref<IndexBuffer> plane_ibo_;
		Ref<Shader> slice_shader_;

		// Crosshair geometry
		Ref<VertexArray> crosshair_vao_;
		Ref<VertexBuffer> crosshair_vbo_;
		Ref<Shader> crosshair_shader_;
	};
}