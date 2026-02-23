#include "pch.h"
#include "MRI/MRISliceViewer.h"
#include <imgui.h>
#include "GUI/GUI.h"
#include "Core/AssetRegistry.h"
#include "Renderer/Renderer.h"

namespace NVMRI {

	void MRISliceViewer::OnAttach(NVMRI::MRIImage* mri_image) {
		mri_image_ = mri_image;
		if (!mri_image_ || !mri_image_->IsValid()) return;

		// Load shaders
		slice_shader_ = CreateRef<Shader>(
			AssetRegistry::Get("SlicePlane.vert"),
			AssetRegistry::Get("SlicePlane.frag")
		);

		crosshair_shader_ = CreateRef<Shader>(
			AssetRegistry::Get("FlatColor.vert"),
			AssetRegistry::Get("FlatColor.frag")
		);

		// Initialize slices
		slices_[0].plane = SlicePlane::Axial;
		slices_[1].plane = SlicePlane::Sagittal;
		slices_[2].plane = SlicePlane::Coronal;

		// Initialize cursor to center of volume
		cursor_voxel_ = glm::vec3(
			mri_image_->dimensions[0] / 2.0f,
			mri_image_->dimensions[1] / 2.0f,
			mri_image_->dimensions[2] / 2.0f
		);

		// Set window/level for T1 images (original hardcoded range that worked well)
		// Original code used val/1500.0f which is equivalent to center=750, width=1500
		window_center_ = 750.0f;
		window_width_ = 1500.0f;

		// Setup quad mesh for slice rendering (-0.5 to +0.5, maps to 0-1 UV)
		float vertices[] = {
			// Position          // TexCoord
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

		// Setup crosshair geometry (4 vertices for 2 lines, dynamic buffer)
		crosshair_vao_ = CreateRef<VertexArray>();
		crosshair_vbo_ = CreateRef<VertexBuffer>(sizeof(float) * 3 * 4); // Dynamic buffer for 4 vertices
		crosshair_vbo_->SetLayout({
			{ ShaderDataType::Float3, "a_Position" }
		});
		crosshair_vao_->AddVertexBuffer(crosshair_vbo_);

		// Create initial textures
		UpdateAllSliceTextures();
	}

	uint32_t MRISliceViewer::GetVaryingDimension(SlicePlane plane) {
		switch (plane) {
			case SlicePlane::Axial:    return 2; // Z varies
			case SlicePlane::Sagittal: return 0; // X varies
			case SlicePlane::Coronal:  return 1; // Y varies
		}
		return 0;
	}

	uint32_t MRISliceViewer::GetSliceIndex(SlicePlane plane) const {
		if (!mri_image_) return 0;
		uint32_t dim = GetVaryingDimension(plane);
		uint32_t maxIdx = mri_image_->dimensions[dim] - 1;
		float coord = (dim == 0) ? cursor_voxel_.x : (dim == 1) ? cursor_voxel_.y : cursor_voxel_.z;
		return static_cast<uint32_t>(glm::clamp(coord, 0.0f, (float)maxIdx));
	}

	void MRISliceViewer::SetCursorVoxel(const glm::vec3& voxel_pos) {
		if (!mri_image_) return;

		glm::vec3 old_cursor = cursor_voxel_;
		cursor_voxel_ = glm::clamp(voxel_pos,
			glm::vec3(0.0f),
			glm::vec3(
				mri_image_->dimensions[0] - 1.0f,
				mri_image_->dimensions[1] - 1.0f,
				mri_image_->dimensions[2] - 1.0f
			)
		);

		// Mark slices that need updating based on which dimension changed
		if ((int)old_cursor.z != (int)cursor_voxel_.z) slices_[0].needs_update = true; // Axial
		if ((int)old_cursor.x != (int)cursor_voxel_.x) slices_[1].needs_update = true; // Sagittal
		if ((int)old_cursor.y != (int)cursor_voxel_.y) slices_[2].needs_update = true; // Coronal
	}

	void MRISliceViewer::SetCursorNormalized(const glm::vec3& normalized_pos) {
		if (!mri_image_) return;
		SetCursorVoxel(glm::vec3(
			normalized_pos.x * (mri_image_->dimensions[0] - 1),
			normalized_pos.y * (mri_image_->dimensions[1] - 1),
			normalized_pos.z * (mri_image_->dimensions[2] - 1)
		));
	}

	glm::vec3 MRISliceViewer::GetCursorNormalized() const {
		if (!mri_image_) return glm::vec3(0.5f);
		return glm::vec3(
			cursor_voxel_.x / (mri_image_->dimensions[0] - 1),
			cursor_voxel_.y / (mri_image_->dimensions[1] - 1),
			cursor_voxel_.z / (mri_image_->dimensions[2] - 1)
		);
	}

	glm::vec2 MRISliceViewer::GetCrosshairUV(SlicePlane plane) const {
		if (!mri_image_) return glm::vec2(0.5f);

		auto& dims = mri_image_->dimensions;
		switch (plane) {
			case SlicePlane::Axial:
				// Axial shows XY plane, crosshair at cursor.x, cursor.y
				return glm::vec2(
					cursor_voxel_.x / (dims[0] - 1),
					cursor_voxel_.y / (dims[1] - 1)
				);
			case SlicePlane::Sagittal:
				// Sagittal shows YZ plane, crosshair at cursor.y, cursor.z
				return glm::vec2(
					cursor_voxel_.y / (dims[1] - 1),
					cursor_voxel_.z / (dims[2] - 1)
				);
			case SlicePlane::Coronal:
				// Coronal shows XZ plane, crosshair at cursor.x, cursor.z
				return glm::vec2(
					cursor_voxel_.x / (dims[0] - 1),
					cursor_voxel_.z / (dims[2] - 1)
				);
		}
		return glm::vec2(0.5f);
	}

	glm::mat4 MRISliceViewer::ComputeSliceTransform(SlicePlane plane) const {
		if (!mri_image_) return glm::mat4(1.0f);

		auto& dims = mri_image_->dimensions;
		auto& spacing = mri_image_->spacing;

		uint32_t sliceIdx = GetSliceIndex(plane);
		glm::vec3 corner00, corner10, corner01, corner11;

		switch (plane) {
			case SlicePlane::Axial:
				// XY plane at Z = sliceIdx
				corner00 = glm::vec3(0,           0,           sliceIdx);
				corner10 = glm::vec3(dims[0] - 1, 0,           sliceIdx);
				corner01 = glm::vec3(0,           dims[1] - 1, sliceIdx);
				corner11 = glm::vec3(dims[0] - 1, dims[1] - 1, sliceIdx);
				break;
			case SlicePlane::Sagittal:
				// YZ plane at X = sliceIdx
				corner00 = glm::vec3(sliceIdx, 0,           0);
				corner10 = glm::vec3(sliceIdx, dims[1] - 1, 0);
				corner01 = glm::vec3(sliceIdx, 0,           dims[2] - 1);
				corner11 = glm::vec3(sliceIdx, dims[1] - 1, dims[2] - 1);
				break;
			case SlicePlane::Coronal:
				// XZ plane at Y = sliceIdx
				corner00 = glm::vec3(0,           sliceIdx, 0);
				corner10 = glm::vec3(dims[0] - 1, sliceIdx, 0);
				corner01 = glm::vec3(0,           sliceIdx, dims[2] - 1);
				corner11 = glm::vec3(dims[0] - 1, sliceIdx, dims[2] - 1);
				break;
		}

		glm::vec3 w00, w10, w01;

		if (use_world_transform_) {
			// Use full voxel-to-world transform (includes direction matrix and LPS conversion)
			auto voxel_to_world = mri_image_->GetVoxelToWorld();
			w00 = glm::vec3(voxel_to_world * glm::vec4(corner00, 1.0f));
			w10 = glm::vec3(voxel_to_world * glm::vec4(corner10, 1.0f));
			w01 = glm::vec3(voxel_to_world * glm::vec4(corner01, 1.0f));
		} else {
			// Axes-aligned mode: just apply spacing, no rotation
			// Center the volume at origin
			glm::vec3 center_offset = glm::vec3(
				dims[0] * spacing[0] / 2.0f,
				dims[1] * spacing[1] / 2.0f,
				dims[2] * spacing[2] / 2.0f
			);
			w00 = glm::vec3(corner00.x * spacing[0], corner00.y * spacing[1], corner00.z * spacing[2]) - center_offset;
			w10 = glm::vec3(corner10.x * spacing[0], corner10.y * spacing[1], corner10.z * spacing[2]) - center_offset;
			w01 = glm::vec3(corner01.x * spacing[0], corner01.y * spacing[1], corner01.z * spacing[2]) - center_offset;
		}

		// Compute local axes from transformed corners
		glm::vec3 right = w10 - w00;
		glm::vec3 up = w01 - w00;
		glm::vec3 normal = glm::normalize(glm::cross(right, up));

		// Center the quad (vertices are -0.5 to +0.5)
		glm::vec3 center = w00 + right * 0.5f + up * 0.5f;

		// Build model matrix
		glm::mat4 model(1.0f);
		model[0] = glm::vec4(right, 0.0f);
		model[1] = glm::vec4(up, 0.0f);
		model[2] = glm::vec4(normal, 0.0f);
		model[3] = glm::vec4(center, 1.0f);

		// Apply manual rotation offset around center
		if (rotation_offset_deg_ != glm::vec3(0.0f)) {
			glm::vec3 rad = glm::radians(rotation_offset_deg_);
			glm::mat4 rot = glm::mat4(1.0f);
			rot = glm::rotate(rot, rad.x, glm::vec3(1, 0, 0));
			rot = glm::rotate(rot, rad.y, glm::vec3(0, 1, 0));
			rot = glm::rotate(rot, rad.z, glm::vec3(0, 0, 1));

			// Rotate around the volume center (translate to origin, rotate, translate back)
			glm::vec3 vol_center = center;
			glm::mat4 toOrigin = glm::translate(glm::mat4(1.0f), -vol_center);
			glm::mat4 fromOrigin = glm::translate(glm::mat4(1.0f), vol_center);
			model = fromOrigin * rot * toOrigin * model;
		}

		// Apply scale factor
		if (scale_factor_ != 1.0f) {
			model = glm::scale(glm::mat4(1.0f), glm::vec3(scale_factor_)) * model;
		}

		return model;
	}

	void MRISliceViewer::UpdateSliceTexture(int i) {
		if (!mri_image_ || i < 0 || i >= 3) return;

		auto& slice = slices_[i];
		uint32_t sliceIdx = GetSliceIndex(slice.plane);

		auto rgbaData = ExtractSliceRGBA8(slice.plane, sliceIdx);

		// Determine texture dimensions based on plane
		uint32_t w = 0, h = 0;
		auto& dims = mri_image_->dimensions;
		switch (slice.plane) {
			case SlicePlane::Axial:    w = dims[0]; h = dims[1]; break; // XY
			case SlicePlane::Sagittal: w = dims[1]; h = dims[2]; break; // YZ
			case SlicePlane::Coronal:  w = dims[0]; h = dims[2]; break; // XZ
		}

		TextureSpecification spec;
		spec.Width = w;
		spec.Height = h;
		spec.Format = ImageFormat::RGBA8;

		slice.texture = CreateRef<Texture>(spec);
		slice.texture->SetData(rgbaData.data(), rgbaData.size());
		slice.needs_update = false;
	}

	void MRISliceViewer::UpdateAllSliceTextures() {
		for (int i = 0; i < 3; i++) {
			UpdateSliceTexture(i);
		}
	}

	std::vector<uint8_t> MRISliceViewer::ExtractSliceRGBA8(SlicePlane plane, uint32_t sliceIdx) {
		auto& dims = mri_image_->dimensions;
		float* buffer = mri_image_->itkImage->GetBufferPointer();

		// Texture dimensions
		uint32_t w = 0, h = 0;
		switch (plane) {
			case SlicePlane::Axial:    w = dims[0]; h = dims[1]; break;
			case SlicePlane::Sagittal: w = dims[1]; h = dims[2]; break;
			case SlicePlane::Coronal:  w = dims[0]; h = dims[2]; break;
		}

		std::vector<uint8_t> data(w * h * 4);

		// Window/level parameters
		float wl_min = window_center_ - window_width_ / 2.0f;
		float wl_max = window_center_ + window_width_ / 2.0f;
		float wl_range = wl_max - wl_min;
		if (wl_range < 0.001f) wl_range = 1.0f; // Avoid division by zero

		for (uint32_t y = 0; y < h; y++) {
			for (uint32_t x = 0; x < w; x++) {
				uint32_t i3d = 0;

				// ITK stores in row-major order: dims[0] is fastest varying
				// Index = i + j*dims[0] + k*dims[0]*dims[1]
				switch (plane) {
					case SlicePlane::Axial:
						// XY plane at Z=sliceIdx: texture(x,y) = voxel(x, y, sliceIdx)
						i3d = x + y * dims[0] + sliceIdx * dims[0] * dims[1];
						break;
					case SlicePlane::Sagittal:
						// YZ plane at X=sliceIdx: texture(x,y) = voxel(sliceIdx, x, y)
						i3d = sliceIdx + x * dims[0] + y * dims[0] * dims[1];
						break;
					case SlicePlane::Coronal:
						// XZ plane at Y=sliceIdx: texture(x,y) = voxel(x, sliceIdx, y)
						i3d = x + sliceIdx * dims[0] + y * dims[0] * dims[1];
						break;
				}

				float val = buffer[i3d];
				// Apply window/level
				float normalized = (val - wl_min) / wl_range;
				uint8_t pixel = static_cast<uint8_t>(glm::clamp(normalized * 255.0f, 0.0f, 255.0f));

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

		// Update textures for slices that changed
		for (int i = 0; i < 3; i++) {
			if (slices_[i].needs_update) {
				UpdateSliceTexture(i);
			}
		}

		// Draw all slices
		for (int i = 0; i < 3; i++) {
			DrawSlice(slices_[i]);
		}

		// Draw crosshairs on top
		if (show_crosshairs_) {
			DrawCrosshairs();
		}
	}

	void MRISliceViewer::DrawSlice(Slice& slice) {
		if (!slice.texture) return;

		RenderCommand cmd;
		cmd.VAOPtr = plane_vao_.get();
		cmd.ShaderPtr = slice_shader_.get();
		cmd.Transform = ComputeSliceTransform(slice.plane);
		cmd.target_viewport = ViewportType::AnatomyViewport;
		cmd.TextureBindings.push_back({ slice.texture->GetRendererID(), 0, nullptr });

		Renderer::Submit(cmd);
	}

	void MRISliceViewer::DrawCrosshairs() {
		if (!mri_image_ || !crosshair_shader_) return;

		// For each slice, draw two lines (horizontal and vertical) at the crosshair position
		for (int i = 0; i < 3; i++) {
			auto& slice = slices_[i];
			glm::mat4 sliceTransform = ComputeSliceTransform(slice.plane);
			glm::vec2 uv = GetCrosshairUV(slice.plane);

			// Convert UV (0-1) to local quad coordinates (-0.5 to +0.5)
			float localX = uv.x - 0.5f;
			float localY = uv.y - 0.5f;

			// Create crosshair lines in local space
			// Small offset along normal to prevent z-fighting
			float z_offset = 0.001f;

			float lines[] = {
				// Horizontal line (full width at localY)
				-0.5f, localY, z_offset,
				 0.5f, localY, z_offset,
				// Vertical line (full height at localX)
				localX, -0.5f, z_offset,
				localX,  0.5f, z_offset,
			};

			crosshair_vbo_->SetData(lines, sizeof(lines));

			RenderCommand cmd;
			cmd.VAOPtr = crosshair_vao_.get();
			cmd.ShaderPtr = crosshair_shader_.get();
			cmd.Transform = sliceTransform;
			cmd.target_viewport = ViewportType::AnatomyViewport;
			cmd.Mode = DrawMode::DRAW_LINES;
			cmd.VertexCount = 4;

			// Set crosshair color via uniform command (u_Color is vec4)
			UniformData colorUniform;
			colorUniform.Type = UniformDataType::FLOAT4;
			colorUniform.Name = "u_Color";
			colorUniform.Data.f4 = glm::vec4(crosshair_color_, 1.0f);
			cmd.UniformCommands.push_back(colorUniform);

			Renderer::Submit(cmd);
		}
	}

	void MRISliceViewer::Render(bool standalone) {
		if (!mri_image_ || !mri_image_->IsValid()) {
			ImGui::TextDisabled("No MRI image loaded");
			return;
		}

		if (ImGui::CollapsingHeader("MRI 3D Slice Viewer", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto& dims = mri_image_->dimensions;

			// Single 3D cursor control
			ImGui::Text("Cursor Position (Voxel):");

			bool cursor_changed = false;
			glm::vec3 cursor = cursor_voxel_;

			ImGui::PushItemWidth(100);
			if (ImGui::SliderFloat("X (Sagittal)", &cursor.x, 0.0f, (float)(dims[0] - 1), "%.0f")) {
				cursor_changed = true;
			}
			ImGui::SameLine();
			ImGui::Text("/ %u", dims[0] - 1);

			if (ImGui::SliderFloat("Y (Coronal)", &cursor.y, 0.0f, (float)(dims[1] - 1), "%.0f")) {
				cursor_changed = true;
			}
			ImGui::SameLine();
			ImGui::Text("/ %u", dims[1] - 1);

			if (ImGui::SliderFloat("Z (Axial)", &cursor.z, 0.0f, (float)(dims[2] - 1), "%.0f")) {
				cursor_changed = true;
			}
			ImGui::SameLine();
			ImGui::Text("/ %u", dims[2] - 1);
			ImGui::PopItemWidth();

			if (cursor_changed) {
				SetCursorVoxel(cursor);
			}

			ImGui::Separator();

			// Window/Level controls
			ImGui::Text("Window/Level:");
			bool wl_changed = false;

			float intensity_range = mri_image_->intensity_max - mri_image_->intensity_min;
			float width_max = std::max(intensity_range, 100.0f); // Ensure reasonable range

			if (ImGui::DragFloat("Center", &window_center_, intensity_range * 0.01f,
				mri_image_->intensity_min, mri_image_->intensity_max, "%.1f")) {
				wl_changed = true;
			}
			if (ImGui::DragFloat("Width", &window_width_, width_max * 0.01f,
				1.0f, width_max, "%.1f")) {
				wl_changed = true;
			}

			// Quick presets
			if (ImGui::Button("T1 Default")) {
				window_center_ = 750.0f;
				window_width_ = 1500.0f;
				wl_changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Percentile")) {
				window_center_ = (mri_image_->intensity_p02 + mri_image_->intensity_p98) / 2.0f;
				window_width_ = std::max(1.0f, mri_image_->intensity_p98 - mri_image_->intensity_p02);
				wl_changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Full Range")) {
				window_center_ = (mri_image_->intensity_min + mri_image_->intensity_max) / 2.0f;
				window_width_ = intensity_range;
				wl_changed = true;
			}

			if (wl_changed) {
				// Mark all slices for update
				for (auto& s : slices_) s.needs_update = true;
			}

			ImGui::Separator();

			// Crosshair settings
			ImGui::Checkbox("Show Crosshairs", &show_crosshairs_);
			if (show_crosshairs_) {
				ImGui::ColorEdit3("Crosshair Color", &crosshair_color_.x);
			}

			ImGui::Separator();

			// Alignment controls
			ImGui::Text("Alignment:");
			if (ImGui::Checkbox("Use World Transform", &use_world_transform_)) {
				// Mark all slices for update when transform mode changes
				for (auto& s : slices_) s.needs_update = true;
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("When enabled, uses the MRI's orientation matrix (may be oblique).\nWhen disabled, slices align to voxel grid axes.");
			}

			ImGui::Text("Manual Rotation Offset:");
			bool rot_changed = false;
			ImGui::PushItemWidth(80);
			if (ImGui::DragFloat("Pitch (X)", &rotation_offset_deg_.x, 0.5f, -180.0f, 180.0f, "%.1f")) rot_changed = true;
			ImGui::SameLine();
			if (ImGui::DragFloat("Yaw (Y)", &rotation_offset_deg_.y, 0.5f, -180.0f, 180.0f, "%.1f")) rot_changed = true;
			ImGui::SameLine();
			if (ImGui::DragFloat("Roll (Z)", &rotation_offset_deg_.z, 0.5f, -180.0f, 180.0f, "%.1f")) rot_changed = true;
			ImGui::PopItemWidth();

			if (ImGui::Button("Reset Rotation")) {
				rotation_offset_deg_ = glm::vec3(0.0f);
			}

			ImGui::SliderFloat("Scale", &scale_factor_, 0.1f, 5.0f, "%.2f");

			ImGui::Separator();

			// Slice info
			const char* names[] = { "Axial (XY)", "Sagittal (YZ)", "Coronal (XZ)" };
			for (int i = 0; i < 3; i++) {
				uint32_t idx = GetSliceIndex(slices_[i].plane);
				uint32_t dim = GetVaryingDimension(slices_[i].plane);
				ImGui::Text("%s: Slice %u / %u", names[i], idx, dims[dim] - 1);
			}

			// Debug info
			if (ImGui::CollapsingHeader("Debug Info")) {
				ImGui::Text("Intensity Range: [%.1f, %.1f]", mri_image_->intensity_min, mri_image_->intensity_max);
				ImGui::Text("Percentiles: P02=%.1f, P98=%.1f", mri_image_->intensity_p02, mri_image_->intensity_p98);
				ImGui::Text("Window: Center=%.1f, Width=%.1f", window_center_, window_width_);
				ImGui::Text("Spacing: %.3f x %.3f x %.3f mm",
					mri_image_->spacing[0], mri_image_->spacing[1], mri_image_->spacing[2]);
			}
		}
	}

} // namespace NVMRI
