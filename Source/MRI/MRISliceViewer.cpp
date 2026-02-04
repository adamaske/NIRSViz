#include "pch.h"
#include "MRI/MRISliceViewer.h"
#include <algorithm>

namespace NVMRI {

	void MRISliceViewer::Initialize(const Ref<MRIImage>& mri_image) {
		NVIZ_PROFILE_FUNCTION();

		mri_image_ = mri_image;

		if (!mri_image_ || !mri_image_->IsValid()) {
			NVIZ_WARN("MRISliceViewer: Cannot initialize - invalid MRI image");
			return;
		}

		// Calculate intensity range from MRI data
		auto buffer = mri_image_->itkImage->GetBufferPointer();
		size_t count = mri_image_->GetVoxelCount();

		intensity_min_ = buffer[0];
		intensity_max_ = buffer[0];
		for (size_t i = 0; i < count; ++i) {
			intensity_min_ = std::min(intensity_min_, buffer[i]);
			intensity_max_ = std::max(intensity_max_, buffer[i]);
		}

		// Initialize default window level
		window_center_ = (intensity_min_ + intensity_max_) / 2.0f;
		window_width_ = intensity_max_ - intensity_min_;

		const auto& dims = mri_image_->dimensions;

		// Axial (XY plane) - slices along Z
		slice_views_[0].orientation = SliceOrientation::Axial;
		slice_views_[0].width = dims[0];
		slice_views_[0].height = dims[1];
		slice_views_[0].max_slice_index = dims[2] - 1;
		slice_views_[0].slice_index = dims[2] / 2;

		// Coronal (XZ plane) - slices along Y
		slice_views_[1].orientation = SliceOrientation::Coronal;
		slice_views_[1].width = dims[0];
		slice_views_[1].height = dims[2];
		slice_views_[1].max_slice_index = dims[1] - 1;
		slice_views_[1].slice_index = dims[1] / 2;

		// Sagittal (YZ plane) - slices along X
		slice_views_[2].orientation = SliceOrientation::Sagittal;
		slice_views_[2].width = dims[1];
		slice_views_[2].height = dims[2];
		slice_views_[2].max_slice_index = dims[0] - 1;
		slice_views_[2].slice_index = dims[0] / 2;

		// Create textures and buffers for each view
		for (auto& view : slice_views_) {
			TextureSpecification spec;
			spec.Width = view.width;
			spec.Height = view.height;
			spec.Format = ImageFormat::RGBA8;
			spec.GenerateMips = false;

			view.texture = CreateRef<Texture>(spec);
			view.pixel_buffer.resize(view.width * view.height * 4); // RGBA
			view.needs_update = true;
		}

		// Initial extraction for all orientations
		for (int i = 0; i < 3; ++i) {
			ExtractSlice(static_cast<SliceOrientation>(i));
		}

		CreateSlicePlaneGeometry();

		NVIZ_INFO("MRISliceViewer: Initialized with dimensions {}x{}x{}",
			dims[0], dims[1], dims[2]);
	}

	void MRISliceViewer::ExtractSlice(SliceOrientation orientation) {
		NVIZ_PROFILE_FUNCTION();

		if (!mri_image_ || !mri_image_->IsValid()) return;

		auto& view = slice_views_[static_cast<int>(orientation)];
		const auto& dims = mri_image_->dimensions;
		float* buffer = mri_image_->itkImage->GetBufferPointer();

		int idx = view.slice_index;

		for (uint32_t row = 0; row < view.height; ++row) {
			for (uint32_t col = 0; col < view.width; ++col) {
				// Calculate 3D index based on orientation
				size_t voxel_idx = 0;
				switch (orientation) {
					case SliceOrientation::Axial:
						// XY plane at Z=idx
						voxel_idx = col + row * dims[0] + idx * dims[0] * dims[1];
						break;
					case SliceOrientation::Coronal:
						// XZ plane at Y=idx
						voxel_idx = col + idx * dims[0] + row * dims[0] * dims[1];
						break;
					case SliceOrientation::Sagittal:
						// YZ plane at X=idx
						voxel_idx = idx + col * dims[0] + row * dims[0] * dims[1];
						break;
				}

				// Get intensity and apply window/level
				float intensity = buffer[voxel_idx];
				float normalized = ApplyWindowLevel(intensity);
				uint8_t gray = static_cast<uint8_t>(normalized * 255.0f);

				// Write RGBA (grayscale with full alpha)
				size_t pixel_idx = (row * view.width + col) * 4;
				view.pixel_buffer[pixel_idx + 0] = gray; // R
				view.pixel_buffer[pixel_idx + 1] = gray; // G
				view.pixel_buffer[pixel_idx + 2] = gray; // B
				view.pixel_buffer[pixel_idx + 3] = 255;  // A
			}
		}

		// Upload to GPU
		view.texture->SetData(view.pixel_buffer.data(),
			static_cast<uint32_t>(view.pixel_buffer.size()));
		view.needs_update = false;
	}

	float MRISliceViewer::ApplyWindowLevel(float intensity) const {
		float lower = window_center_ - window_width_ / 2.0f;
		float upper = window_center_ + window_width_ / 2.0f;

		if (intensity <= lower) return 0.0f;
		if (intensity >= upper) return 1.0f;

		return (intensity - lower) / window_width_;
	}

	void MRISliceViewer::SetSliceIndex(SliceOrientation orientation, int index) {
		auto& view = slice_views_[static_cast<int>(orientation)];
		index = std::clamp(index, 0, view.max_slice_index);

		if (view.slice_index != index) {
			view.slice_index = index;
			view.needs_update = true;
			ExtractSlice(orientation);
		}
	}

	int MRISliceViewer::GetSliceIndex(SliceOrientation orientation) const {
		return slice_views_[static_cast<int>(orientation)].slice_index;
	}

	int MRISliceViewer::GetMaxSliceIndex(SliceOrientation orientation) const {
		return slice_views_[static_cast<int>(orientation)].max_slice_index;
	}

	uint32_t MRISliceViewer::GetTextureID(SliceOrientation orientation) const {
		return slice_views_[static_cast<int>(orientation)].texture->GetRendererID();
	}

	glm::vec2 MRISliceViewer::GetSliceSize(SliceOrientation orientation) const {
		const auto& view = slice_views_[static_cast<int>(orientation)];
		return { static_cast<float>(view.width), static_cast<float>(view.height) };
	}

	void MRISliceViewer::SetWindowLevel(float center, float width) {
		if (window_center_ != center || window_width_ != width) {
			window_center_ = center;
			window_width_ = std::max(width, 1.0f); // Prevent division by zero

			// Re-extract all slices with new window/level
			for (int i = 0; i < 3; ++i) {
				ExtractSlice(static_cast<SliceOrientation>(i));
			}
		}
	}

	SliceView& MRISliceViewer::GetSliceView(SliceOrientation orientation) {
		return slice_views_[static_cast<int>(orientation)];
	}

	const SliceView& MRISliceViewer::GetSliceView(SliceOrientation orientation) const {
		return slice_views_[static_cast<int>(orientation)];
	}

	void MRISliceViewer::CreateSlicePlaneGeometry() {
		// Unit quad in XY plane (will be transformed per orientation)
		// Position (3) + TexCoord (2)
		float vertices[] = {
			// Position          TexCoord
			0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
			0.0f, 1.0f, 0.0f,   0.0f, 1.0f
		};

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		plane_vao_ = CreateRef<VertexArray>();
		plane_vao_->Bind();

		plane_vbo_ = CreateRef<VertexBuffer>(vertices, sizeof(vertices));

		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		plane_vbo_->SetLayout(layout);
		plane_vao_->AddVertexBuffer(plane_vbo_);

		plane_ibo_ = CreateRef<IndexBuffer>(indices, 6);
		plane_vao_->SetIndexBuffer(plane_ibo_);

		plane_vao_->Unbind();
	}

	glm::mat4 MRISliceViewer::GetSlicePlaneTransform(SliceOrientation orientation) const {
		if (!mri_image_ || !mri_image_->IsValid())
			return glm::mat4(1.0f);

		const auto& dims = mri_image_->dimensions;
		const auto& spacing = mri_image_->spacing;
		const auto& origin = mri_image_->origin;
		const auto& view = slice_views_[static_cast<int>(orientation)];

		glm::mat4 transform = glm::mat4(1.0f);

		// Physical dimensions
		float sx = dims[0] * static_cast<float>(spacing[0]);
		float sy = dims[1] * static_cast<float>(spacing[1]);
		float sz = dims[2] * static_cast<float>(spacing[2]);

		switch (orientation) {
			case SliceOrientation::Axial: {
				// XY plane at Z position
				float z = static_cast<float>(origin[2]) + view.slice_index * static_cast<float>(spacing[2]);
				transform = glm::translate(glm::mat4(1.0f),
					glm::vec3(static_cast<float>(origin[0]), static_cast<float>(origin[1]), z));
				transform = glm::scale(transform, glm::vec3(sx, sy, 1.0f));
				break;
			}
			case SliceOrientation::Coronal: {
				// XZ plane at Y position
				float y = static_cast<float>(origin[1]) + view.slice_index * static_cast<float>(spacing[1]);
				transform = glm::translate(glm::mat4(1.0f),
					glm::vec3(static_cast<float>(origin[0]), y, static_cast<float>(origin[2])));
				transform = glm::rotate(transform,
					glm::radians(-90.0f), glm::vec3(1, 0, 0));
				transform = glm::scale(transform, glm::vec3(sx, sz, 1.0f));
				break;
			}
			case SliceOrientation::Sagittal: {
				// YZ plane at X position
				float x = static_cast<float>(origin[0]) + view.slice_index * static_cast<float>(spacing[0]);
				transform = glm::translate(glm::mat4(1.0f),
					glm::vec3(x, static_cast<float>(origin[1]), static_cast<float>(origin[2])));
				transform = glm::rotate(transform,
					glm::radians(90.0f), glm::vec3(0, 1, 0));
				transform = glm::scale(transform, glm::vec3(sy, sz, 1.0f));
				break;
			}
		}

		return transform;
	}

	void MRISliceViewer::SetupSlicePlaneRendering(Ref<Shader> shader) {
		slice_plane_shader_ = shader;
	}

	void MRISliceViewer::RenderSlicePlanes(ViewportType target_viewport, float opacity) {
		if (!slice_plane_shader_ || !plane_vao_) return;

		for (int i = 0; i < 3; ++i) {
			auto orientation = static_cast<SliceOrientation>(i);
			const auto& view = slice_views_[i];

			RenderCommand cmd;
			cmd.ShaderPtr = slice_plane_shader_.get();
			cmd.VAOPtr = plane_vao_.get();
			cmd.target_viewport = target_viewport;
			cmd.Transform = GetSlicePlaneTransform(orientation);
			cmd.Mode = DRAW_ELEMENTS;

			// Bind slice texture
			TextureBinding tex_binding;
			tex_binding.TexturePtr = view.texture.get();
			tex_binding.Slot = 0;
			cmd.TextureBindings = { tex_binding };

			// Set opacity uniform
			UniformData opacity_uniform;
			opacity_uniform.Type = UniformDataType::FLOAT1;
			opacity_uniform.Name = "u_Opacity";
			opacity_uniform.Data.f1 = opacity;
			cmd.UniformCommands = { opacity_uniform };

			Renderer::Submit(cmd);
		}
	}

}
