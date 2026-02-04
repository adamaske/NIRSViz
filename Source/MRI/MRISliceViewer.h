#pragma once

#include "Core/Base.h"
#include "MRI/MRIImage.h"
#include "Renderer/Renderable/Texture.h"
#include "Renderer/Buffer/VertexArray.h"
#include "Renderer/Buffer/VertexBuffer.h"
#include "Renderer/Buffer/IndexBuffer.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderer.h"
#include <array>

namespace NVMRI {

	enum class SliceOrientation {
		Axial = 0,    // XY plane, viewing along Z
		Coronal = 1,  // XZ plane, viewing along Y
		Sagittal = 2  // YZ plane, viewing along X
	};

	struct SliceView {
		SliceOrientation orientation;
		int slice_index = 0;
		int max_slice_index = 0;

		Ref<Texture> texture;
		std::vector<uint8_t> pixel_buffer;

		uint32_t width = 0;
		uint32_t height = 0;
		bool needs_update = true;
	};

	class MRISliceViewer {
	public:
		MRISliceViewer() = default;
		~MRISliceViewer() = default;

		// Initialize with MRI image
		void Initialize(const Ref<MRIImage>& mri_image);

		// Set slice index for an orientation
		void SetSliceIndex(SliceOrientation orientation, int index);
		int GetSliceIndex(SliceOrientation orientation) const;
		int GetMaxSliceIndex(SliceOrientation orientation) const;

		// Get texture for ImGui rendering
		uint32_t GetTextureID(SliceOrientation orientation) const;
		glm::vec2 GetSliceSize(SliceOrientation orientation) const;

		// Intensity windowing
		void SetWindowLevel(float center, float width);
		float GetWindowCenter() const { return window_center_; }
		float GetWindowWidth() const { return window_width_; }
		float GetIntensityMin() const { return intensity_min_; }
		float GetIntensityMax() const { return intensity_max_; }

		// 3D slice plane rendering
		void SetupSlicePlaneRendering(Ref<Shader> shader);
		void RenderSlicePlanes(ViewportType target_viewport, float opacity);

		// Access to slice views
		SliceView& GetSliceView(SliceOrientation orientation);
		const SliceView& GetSliceView(SliceOrientation orientation) const;

	private:
		void ExtractSlice(SliceOrientation orientation);
		float ApplyWindowLevel(float intensity) const;
		void CreateSlicePlaneGeometry();
		glm::mat4 GetSlicePlaneTransform(SliceOrientation orientation) const;

		Ref<MRIImage> mri_image_;
		std::array<SliceView, 3> slice_views_;

		// Global intensity windowing
		float window_center_ = 0.5f;
		float window_width_ = 1.0f;
		float intensity_min_ = 0.0f;
		float intensity_max_ = 1.0f;

		// 3D slice plane geometry
		Ref<VertexArray> plane_vao_;
		Ref<VertexBuffer> plane_vbo_;
		Ref<IndexBuffer> plane_ibo_;
		Ref<Shader> slice_plane_shader_;
	};

}
