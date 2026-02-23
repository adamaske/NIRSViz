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

		// In box-edge mode the quad is fixed at the volume boundary (index 0) so all
		// three planes meet at a single box corner.  The texture content is unchanged —
		// it still shows the cursor's slice — only the 3D quad position is locked.
		uint32_t posIdx = box_edge_mode_ ? 0u : sliceIdx;

		glm::vec3 corner00, corner10, corner01, corner11;

		switch (plane) {
			case SlicePlane::Axial:
				// XY plane at Z = posIdx
				corner00 = glm::vec3(0,           0,           posIdx);
				corner10 = glm::vec3(dims[0] - 1, 0,           posIdx);
				corner01 = glm::vec3(0,           dims[1] - 1, posIdx);
				corner11 = glm::vec3(dims[0] - 1, dims[1] - 1, posIdx);
				break;
			case SlicePlane::Sagittal:
				// YZ plane at X = posIdx
				corner00 = glm::vec3(posIdx, 0,           0);
				corner10 = glm::vec3(posIdx, dims[1] - 1, 0);
				corner01 = glm::vec3(posIdx, 0,           dims[2] - 1);
				corner11 = glm::vec3(posIdx, dims[1] - 1, dims[2] - 1);
				break;
			case SlicePlane::Coronal:
				// XZ plane at Y = posIdx
				corner00 = glm::vec3(0,           posIdx, 0);
				corner10 = glm::vec3(dims[0] - 1, posIdx, 0);
				corner01 = glm::vec3(0,           posIdx, dims[2] - 1);
				corner11 = glm::vec3(dims[0] - 1, posIdx, dims[2] - 1);
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
			// Axis-aligned mode with anatomically-correct Y-up placement.
			//
			// The naive approach of mapping voxel(X,Y,Z) → world(X,Y,Z) and then applying
			// a Rx(-90°) rotation offset to fix orientation ONLY rotates the quad's
			// orientation vectors; it cannot move the quad's center, so slices end up
			// sliding along the wrong world axis when the cursor changes.
			//
			// The correct mapping bakes the anatomical convention directly into the
			// corner positions:
			//   voxel X  →  world  X   (left-right)
			//   voxel Z  →  world  Y   (superior-inferior; voxel Z is brain "up")
			//   voxel Y  →  world -Z   (anterior-posterior; negated so anterior = +Z)
			//
			// With this mapping each plane's center naturally slides along the right axis:
			//   Axial    center.y = sliceIdx*sz - halfZ   ✓ moves up/down
			//   Coronal  center.z = halfY - sliceIdx*sy   ✓ moves front/back
			//   Sagittal center.x = sliceIdx*sx - halfX   ✓ moves left/right
			//
			float halfX = dims[0] * (float)spacing[0] * 0.5f;
			float halfY = dims[1] * (float)spacing[1] * 0.5f;
			float halfZ = dims[2] * (float)spacing[2] * 0.5f;

			// Converts a voxel coordinate to its anatomical world position
			auto V = [&](float vx, float vy, float vz) -> glm::vec3 {
				return glm::vec3(
					vx * (float)spacing[0] - halfX,   // world X  ← voxel X
					vz * (float)spacing[2] - halfZ,   // world Y  ← voxel Z
					halfY - vy * (float)spacing[1]    // world Z  ← -voxel Y
				);
			};

			switch (plane) {
				case SlicePlane::Axial:
					// Constant voxel Z = posIdx; voxel X and Y fill the quad
					w00 = V(0,           0,           posIdx);
					w10 = V(dims[0] - 1, 0,           posIdx);
					w01 = V(0,           dims[1] - 1, posIdx);
					break;
				case SlicePlane::Sagittal:
					// Constant voxel X = posIdx; voxel Y and Z fill the quad
					w00 = V(posIdx, 0,           0);
					w10 = V(posIdx, dims[1] - 1, 0);
					w01 = V(posIdx, 0,           dims[2] - 1);
					break;
				case SlicePlane::Coronal:
					// Constant voxel Y = posIdx; voxel X and Z fill the quad
					w00 = V(0,           posIdx, 0);
					w10 = V(dims[0] - 1, posIdx, 0);
					w01 = V(0,           posIdx, dims[2] - 1);
					break;
			}
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

		// Coronal quad faces away from the camera due to winding order; reverse
		// the winding by negating the right (col 0) and normal (col 2) vectors.
		// This flips the face without inverting the up direction, so the image
		// stays right-side up. The center (column 3) is unchanged.
		if (plane == SlicePlane::Coronal) {
			model[0] = -model[0];
			model[2] = -model[2];
		}

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

			ImGui::Checkbox("Box Edge Mode", &box_edge_mode_);
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip(
					"Locks each slice plane at the volume boundary (index 0) so all three\n"
					"planes meet at one corner of the bounding box.\n"
					"Useful with orthographic camera + transparent brain to align anatomy.\n"
					"The displayed slice content still follows the cursor."
				);
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

	void MRISliceViewer::Render2DViewer(bool standalone) {
		if (!mri_image_ || !mri_image_->IsValid()) {
			ImGui::TextDisabled("No MRI image loaded");
			return;
		}

		auto& dims = mri_image_->dimensions;

		// ---- Global toolbar ----
		if (ImGui::Button("Reset All")) {
			SetCursorVoxel(glm::vec3(dims[0] * 0.5f, dims[1] * 0.5f, dims[2] * 0.5f));
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Reset cursor to volume center");
		ImGui::SameLine();
		ImGui::TextDisabled("Drag: crosshair   Scroll: step slice");

		ImGui::Separator();

		// ---- Layout metrics ----
		const float col_sp   = ImGui::GetStyle().ItemSpacing.x;
		const float frame_hs = ImGui::GetFrameHeightWithSpacing();
		const float line_hs  = ImGui::GetTextLineHeightWithSpacing();

		ImVec2 avail = ImGui::GetContentRegionAvail();
		float panel_w = std::max(10.0f, (avail.x - col_sp * 2.0f) / 3.0f);

		// Texture pixel dimensions per plane
		// [plane][0]=tex_w, [plane][1]=tex_h
		uint32_t tex_dims[3][2] = {
			{ dims[0], dims[1] }, // Axial:    XY
			{ dims[1], dims[2] }, // Sagittal: YZ
			{ dims[0], dims[2] }, // Coronal:  XZ
		};

		// All panels share the same image height so rows stay strictly aligned.
		// Compute the tallest ideal height across all planes (fitting in panel_w),
		// then clamp to what fits in the window.
		float overhead = line_hs           // header row
		               + frame_hs          // step-buttons row
		               + frame_hs;         // slice-slider row
		float img_max_h = std::max(50.0f, avail.y - overhead);

		float img_h = 0.0f;
		for (int i = 0; i < 3; i++) {
			float ah = (float)tex_dims[i][1] / (float)tex_dims[i][0];
			img_h = std::max(img_h, panel_w * ah);
		}
		img_h = std::min(img_h, img_max_h);

		// Each panel may be narrower than panel_w to preserve aspect ratio
		float img_w[3];
		for (int i = 0; i < 3; i++) {
			float aw = (float)tex_dims[i][0] / (float)tex_dims[i][1];
			img_w[i] = std::min(panel_w, img_h * aw);
			if (img_w[i] < 1.0f) img_w[i] = 1.0f;
		}

		const SlicePlane  planes[3]       = { SlicePlane::Axial, SlicePlane::Sagittal, SlicePlane::Coronal };
		const char* const plane_labels[3] = { "AXIAL", "SAGITTAL", "CORONAL" };
		// Human label for the axis that varies when stepping through this plane
		const char* const vary_labels[3]  = { "Z", "X", "Y" };

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		for (int i = 0; i < 3; i++) {
			if (i > 0) ImGui::SameLine(0.0f, col_sp);
			ImGui::PushID(i);
			ImGui::BeginGroup();

			auto& slice      = slices_[i];
			uint32_t sl_idx  = GetSliceIndex(planes[i]);
			uint32_t vary_d  = GetVaryingDimension(planes[i]);
			uint32_t sl_max  = dims[vary_d] - 1;

			// ---- Row 1: header label (same height for all: one text line) ----
			ImGui::TextColored(ImVec4(0.55f, 0.85f, 1.0f, 1.0f), "%s", plane_labels[i]);
			ImGui::SameLine();
			ImGui::TextDisabled("%s: %u/%u", vary_labels[i], sl_idx, sl_max);

			// ---- Row 2: image (all panels same img_h) ----
			ImVec2 img_size(img_w[i], img_h);

			if (slice.texture) {
				// Flip Y: OpenGL stores rows bottom-up; uv_min=(0,1) uv_max=(1,0)
				// displays it with increasing voxel indices going upward (radiological)
				ImGui::Image(
					(ImTextureID)(intptr_t)slice.texture->GetRendererID(),
					img_size,
					ImVec2(0.0f, 1.0f),
					ImVec2(1.0f, 0.0f)
				);

				ImVec2 r_min = ImGui::GetItemRectMin();
				ImVec2 r_max = ImGui::GetItemRectMax();

				// Subtle border
				draw_list->AddRect(r_min, r_max, IM_COL32(70, 70, 70, 255));

				bool hovered = ImGui::IsItemHovered();

				// Left-drag: reposition in-plane cursor
				if (hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
					ImVec2 mp = ImGui::GetMousePos();
					float ux = glm::clamp((mp.x - r_min.x) / (r_max.x - r_min.x), 0.0f, 1.0f);
					float uy = glm::clamp(1.0f - (mp.y - r_min.y) / (r_max.y - r_min.y), 0.0f, 1.0f);
					glm::vec3 nc = cursor_voxel_;
					switch (planes[i]) {
						case SlicePlane::Axial:    nc.x = ux*(dims[0]-1); nc.y = uy*(dims[1]-1); break;
						case SlicePlane::Sagittal: nc.y = ux*(dims[1]-1); nc.z = uy*(dims[2]-1); break;
						case SlicePlane::Coronal:  nc.x = ux*(dims[0]-1); nc.z = uy*(dims[2]-1); break;
					}
					SetCursorVoxel(nc);
				}

				// Scroll wheel: step through the orthogonal axis
				if (hovered && ImGui::GetIO().MouseWheel != 0.0f) {
					float wheel = ImGui::GetIO().MouseWheel;
					glm::vec3 nc = cursor_voxel_;
					switch (planes[i]) {
						case SlicePlane::Axial:    nc.z = glm::clamp(nc.z - wheel, 0.0f, (float)(dims[2]-1)); break;
						case SlicePlane::Sagittal: nc.x = glm::clamp(nc.x - wheel, 0.0f, (float)(dims[0]-1)); break;
						case SlicePlane::Coronal:  nc.y = glm::clamp(nc.y - wheel, 0.0f, (float)(dims[1]-1)); break;
					}
					SetCursorVoxel(nc);
				}

				// 2D crosshairs drawn via ImDrawList (no separate OpenGL submission)
				if (show_crosshairs_) {
					glm::vec2 ch = GetCrosshairUV(planes[i]);
					float sx = r_min.x + ch.x * (r_max.x - r_min.x);
					float sy = r_min.y + (1.0f - ch.y) * (r_max.y - r_min.y);
					ImU32 col = IM_COL32(
						(int)(crosshair_color_.r * 255),
						(int)(crosshair_color_.g * 255),
						(int)(crosshair_color_.b * 255),
						210
					);
					draw_list->PushClipRect(r_min, r_max, true);
					draw_list->AddLine({ r_min.x, sy }, { r_max.x, sy }, col, 1.0f);
					draw_list->AddLine({ sx, r_min.y }, { sx, r_max.y }, col, 1.0f);
					draw_list->PopClipRect();
				}

				// Hover tooltip: current voxel position
				if (hovered) {
					ImGui::BeginTooltip();
					ImGui::Text("Voxel  x=%.0f  y=%.0f  z=%.0f",
						cursor_voxel_.x, cursor_voxel_.y, cursor_voxel_.z);
					ImGui::EndTooltip();
				}
			} else {
				// Placeholder keeps layout stable while textures are not yet ready
				ImGui::Dummy(img_size);
			}

			// ---- Row 3: step buttons + center (all SmallButton = same height) ----
			if (ImGui::SmallButton(" < ")) {
				glm::vec3 nc = cursor_voxel_;
				switch (planes[i]) {
					case SlicePlane::Axial:    nc.z = glm::clamp(nc.z - 1.0f, 0.0f, (float)(dims[2]-1)); break;
					case SlicePlane::Sagittal: nc.x = glm::clamp(nc.x - 1.0f, 0.0f, (float)(dims[0]-1)); break;
					case SlicePlane::Coronal:  nc.y = glm::clamp(nc.y - 1.0f, 0.0f, (float)(dims[1]-1)); break;
				}
				SetCursorVoxel(nc);
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Previous slice");
			ImGui::SameLine(0.0f, 1.0f);
			if (ImGui::SmallButton(" > ")) {
				glm::vec3 nc = cursor_voxel_;
				switch (planes[i]) {
					case SlicePlane::Axial:    nc.z = glm::clamp(nc.z + 1.0f, 0.0f, (float)(dims[2]-1)); break;
					case SlicePlane::Sagittal: nc.x = glm::clamp(nc.x + 1.0f, 0.0f, (float)(dims[0]-1)); break;
					case SlicePlane::Coronal:  nc.y = glm::clamp(nc.y + 1.0f, 0.0f, (float)(dims[1]-1)); break;
				}
				SetCursorVoxel(nc);
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Next slice");
			ImGui::SameLine(0.0f, 6.0f);
			if (ImGui::SmallButton("Center")) {
				glm::vec3 nc = cursor_voxel_;
				switch (planes[i]) {
					case SlicePlane::Axial:    nc.x = dims[0]*0.5f; nc.y = dims[1]*0.5f; break;
					case SlicePlane::Sagittal: nc.y = dims[1]*0.5f; nc.z = dims[2]*0.5f; break;
					case SlicePlane::Coronal:  nc.x = dims[0]*0.5f; nc.z = dims[2]*0.5f; break;
				}
				SetCursorVoxel(nc);
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Center cursor on this plane");

			// ---- Row 4: slice slider (width matches image) ----
			float sv = (float)sl_idx;
			ImGui::SetNextItemWidth(img_w[i]);
			char lbl[16];
			snprintf(lbl, sizeof(lbl), "##sl%d", i);
			if (ImGui::SliderFloat(lbl, &sv, 0.0f, (float)sl_max, "%.0f")) {
				glm::vec3 nc = cursor_voxel_;
				switch (planes[i]) {
					case SlicePlane::Axial:    nc.z = sv; break;
					case SlicePlane::Sagittal: nc.x = sv; break;
					case SlicePlane::Coronal:  nc.y = sv; break;
				}
				SetCursorVoxel(nc);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s slice %u / %u", vary_labels[i], sl_idx, sl_max);

			ImGui::EndGroup();
			ImGui::PopID();
		}
	}

} // namespace NVMRI
