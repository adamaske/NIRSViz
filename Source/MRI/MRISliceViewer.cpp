#include "pch.h"
#include "MRI/MRISliceViewer.h"
#include <imgui.h>
#include "GUI/GUI.h"
#include "Core/AssetRegistry.h"

namespace NVMRI {

	void MRISliceViewer::OnAttach(NVMRI::MRIImage* mri_image) {
		mri_image_ = mri_image;
		if (!mri_image_ || !mri_image_->IsValid()) return;

		slice_shader_ = CreateRef<Shader>(
			AssetRegistry::Get("SlicePlane.vert"),
			AssetRegistry::Get("SlicePlane.frag")
		);
		// 1. Initialize Slices
		slices_.resize(3);
		// MRI Dimensions for scaling
		float dx = (float)mri_image_->dimensions[0];
		float dy = (float)mri_image_->dimensions[1];
		float dz = (float)mri_image_->dimensions[2];
		float maxDim = std::max({ dx, dy, dz });
		

		// 1. Axial (XY Plane - moves along Z)
		slices_[0].plane = SlicePlane::Axial;
		slices_[0].transform.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		slices_[0].transform.SetScale(glm::vec3(dx / maxDim, dy / maxDim, 1.0f) * image_scalar_);

		// 2. Sagittal (YZ Plane - moves along X)
		slices_[1].plane = SlicePlane::Sagittal;
		slices_[1].transform.SetRotation(glm::vec3(0.0f, 90.0f, 0.0f));
		slices_[1].transform.SetScale(glm::vec3(dz / maxDim, dy / maxDim, 1.0f) * image_scalar_);

		// 3. Coronal (XZ Plane - moves along Y)
		slices_[2].plane = SlicePlane::Coronal;
		slices_[2].transform.SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
		slices_[2].transform.SetScale(glm::vec3(dx / maxDim, dz / maxDim, 1.0f) * image_scalar_);

		// 2. Setup Mesh (A simple 1x1 quad)
		float vertices[] = {
			-0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,  0.0f, 1.0f
		};
		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

		plane_vao_ = CreateRef<VertexArray>();
		plane_vbo_ = CreateRef<VertexBuffer>(vertices, sizeof(vertices));
		plane_vbo_->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
			});
		plane_ibo_ = CreateRef<IndexBuffer>(indices, 6);
		plane_vao_->AddVertexBuffer(plane_vbo_);
		plane_vao_->SetIndexBuffer(plane_ibo_);

		// 3. Create initial textures
		for (int i = 0; i < 3; i++) {
			UpdateSliceTexture(i);
		}
	}

	void MRISliceViewer::UpdateSliceTexture(int i) {
		auto& slice = slices_[i];
		uint32_t dimIdx = (int)slice.plane;
		uint32_t sliceIdx = (uint32_t)(slice.position * (mri_image_->dimensions[dimIdx] - 1));

		auto rgbaData = ExtractSliceRGBA8(slice.plane, sliceIdx);

		// Determine Texture Dimensions
		uint32_t w = 0, h = 0;
		if (slice.plane == SlicePlane::Axial) { w = mri_image_->dimensions[0]; h = mri_image_->dimensions[1]; }
		if (slice.plane == SlicePlane::Sagittal) { w = mri_image_->dimensions[1]; h = mri_image_->dimensions[2]; }
		if (slice.plane == SlicePlane::Coronal) { w = mri_image_->dimensions[0]; h = mri_image_->dimensions[2]; }

		TextureSpecification spec;
		spec.Width = w;
		spec.Height = h;
		spec.Format = ImageFormat::RGBA8;

		slice.texture = CreateRef<Texture>(spec);
		slice.texture->SetData(rgbaData.data(), rgbaData.size());
	}

	std::vector<uint8_t> MRISliceViewer::ExtractSliceRGBA8(SlicePlane plane, uint32_t sliceIdx) {
		auto& dims = mri_image_->dimensions;
		float* buffer = mri_image_->itkImage->GetBufferPointer();

		uint32_t w = 0, h = 0;
		if (plane == SlicePlane::Axial) { w = dims[0]; h = dims[1]; }
		if (plane == SlicePlane::Sagittal) { w = dims[1]; h = dims[2]; }
		if (plane == SlicePlane::Coronal) { w = dims[0]; h = dims[2]; }

		std::vector<uint8_t> data(w * h * 4);

		for (uint32_t y = 0; y < h; y++) {
			for (uint32_t x = 0; x < w; x++) {
				uint32_t i3d = 0;
				if (plane == SlicePlane::Axial)    i3d = x + y * dims[0] + sliceIdx * dims[0] * dims[1];
				if (plane == SlicePlane::Sagittal) i3d = sliceIdx + x * dims[0] + y * dims[0] * dims[1];
				if (plane == SlicePlane::Coronal)  i3d = x + sliceIdx * dims[0] + y * dims[0] * dims[1];

				float val = buffer[i3d];
				uint8_t pixel = (uint8_t)glm::clamp((val / 1500.0f) * 255.0f, 0.0f, 255.0f);

				uint32_t i2d = (x + y * w) * 4;
				data[i2d + 0] = pixel; // R
				data[i2d + 1] = pixel; // G
				data[i2d + 2] = pixel; // B
				data[i2d + 3] = 255;   // A
			}
		}
		return data;
	}

	void MRISliceViewer::OnUpdate(DeltaTime dt) {
		if (!mri_image_) return;

		for (int i = 0; i < 3; i++) {
			auto& slice = slices_[i];
			float offset = slice.position - 0.5f; // Map 0->1 to -0.5->0.5

			glm::vec3 pos(0.0f);
			if (slice.plane == SlicePlane::Axial)    pos.z = offset;
			if (slice.plane == SlicePlane::Sagittal) pos.x = offset;
			if (slice.plane == SlicePlane::Coronal)  pos.y = offset;

			slice.transform.SetPosition(pos);
			DrawSlice(slice);
		}
	}

	void MRISliceViewer::Render(bool standalone) {
		if (ImGui::CollapsingHeader("MRI 2D Slice Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Image Scalar", &image_scalar_, 0.1f, 100.0f);
			const char* names[] = { "Axial (XY)", "Sagittal (YZ)", "Coronal (XZ)" };
			for (int i = 0; i < 3; i++) {
				if (ImGui::TreeNode(names[i])) {
					if (ImGui::SliderFloat("Position", &slices_[i].position, 0.0f, 1.0f)) {
						UpdateSliceTexture(i); // Update GPU texture on slider move
					}
					GUI::RenderTransformSettings(slices_[i].transform);
					ImGui::TreePop();
				}
			}
		}
	}

	void MRISliceViewer::DrawSlice(Slice& slice) {
		if (!slice.texture) return;

		RenderCommand cmd;
		cmd.VAOPtr = plane_vao_.get();
		cmd.ShaderPtr = slice_shader_.get();
		cmd.Transform = slice.transform.GetMatrix();
		cmd.target_viewport = ViewportType::AnatomyViewport;

		cmd.TextureBindings.push_back({ slice.texture->GetRendererID(), 0, nullptr });

		Renderer::Submit(cmd);
	}
}