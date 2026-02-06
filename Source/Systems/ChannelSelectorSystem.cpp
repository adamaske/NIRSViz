#include "pch.h"
#include "Systems/ChannelSelectorSystem.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>

#include "Core/Input.h"
#include "Core/AssetManager.h"
#include "Events/MouseCodes.h"
#include "Events/KeyCodes.h"
#include "Events/EventBus.h"

// OpenGL includes (adjust based on your setup)
#include <glad/glad.h>

void ChannelSelectorSystem::OnAttach()
{
	// Create OpenGL texture
	glGenTextures(1, &texture_id_);
	glBindTexture(GL_TEXTURE_2D, texture_id_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	auto snirf = AssetManager::Get<SNIRF>("SNIRF");
	channel_visuals_.clear();
	selected_channels_.clear();

	const auto& probe = snirf->GetProbe();

	channels_ = probe.channels;
	sources_ = probe.sources;
	detectors_ = probe.detectors;

	// Build channel visuals
	for (auto& [id, channel] : channels_) {
		auto& source = sources_[channel.source_id];
		auto& detector = detectors_[channel.detector_id];

		channel_visuals_[id] = Channel2DVisual{
			glm::vec2(source.position_2D.x, source.position_2D.y),
			glm::vec2(detector.position_2D.x, detector.position_2D.y),
			id
		};
	}

	// Auto-fit the view
	FitViewToData();

	// Select all by default
	SelectAllChannels();
	// Subscribe to SNIRF loaded event
}

void ChannelSelectorSystem::OnDetach()
{
	if (texture_id_) {
		glDeleteTextures(1, &texture_id_);
		texture_id_ = 0;
	}
}

static bool initial_selection = false;
void ChannelSelectorSystem::OnUpdate(DeltaTime dt)
{
	if (!initial_selection && !channels_.empty()) {
		auto snrirf = snirf_provider_.GetLoadedSNIRF();

		SelectAllChannels();
		initial_selection = true;
	}

	// Render the scene to our pixel buffer
	if(dirty_)
		RenderToBuffer();

	// Upload to GPU texture
	UpdateTexture();
}

void ChannelSelectorSystem::OnGUIRender()
{
	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
	ImGui::Begin("Channel Selector 2D");

	// Settings panel
	if (ImGui::CollapsingHeader("Settings")) {
		ImGui::SliderFloat("World Scale", &config_.settings.world_scale, 10.0f, 200.0f);
		ImGui::SliderFloat("Source Radius", &config_.settings.source_radius, 2.0f, 20.0f);
		ImGui::SliderFloat("Detector Radius", &config_.settings.detector_radius, 2.0f, 20.0f);
		ImGui::SliderFloat("Channel Width", &config_.settings.channel_width, 1.0f, 10.0f);
		ImGui::SliderFloat("Zoom", &config_.zoom_level, 0.1f, 5.0f);

		if (ImGui::Button("Reset View")) {
			FitViewToData();
		}

		ImGui::SeparatorText("Controls");
		ImGui::Text("Mouse Wheel: Zoom");
		ImGui::Text("Middle Mouse: Pan");
		ImGui::Text("Left Click: Select Channel");
		ImGui::Text("Ctrl + Click: Multi-select");
	}

	// Selection buttons
	if (ImGui::Button("Select All")) SelectAllChannels();
	ImGui::SameLine();
	if (ImGui::Button("Clear Selection")) ClearSelection();

	ImGui::Text("Selected Channels: %zu", selected_channels_.size());

	// Get viewport info
	ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
	viewport_size_ = { viewport_panel_size.x, viewport_panel_size.y };

	// Resize buffer if needed
	if (image_buffer_.Width != (uint32_t)viewport_panel_size.x ||
		image_buffer_.Height != (uint32_t)viewport_panel_size.y)
	{
		if (viewport_panel_size.x > 0 && viewport_panel_size.y > 0) {
			image_buffer_.Resize((uint32_t)viewport_panel_size.x, (uint32_t)viewport_panel_size.y);
		}
	}

	// Get mouse position
	auto window_pos = ImGui::GetCursorScreenPos();
	auto [mx, my] = ImGui::GetMousePos();
	mouse_position_ = { mx - window_pos.x, my - window_pos.y };

	viewport_focused_ = ImGui::IsWindowFocused();
	viewport_hovered_ = ImGui::IsWindowHovered();

	// Display the texture
	if (texture_id_) {
		ImGui::Image(
			(void*)(intptr_t)texture_id_,
			viewport_panel_size,
			ImVec2(0, 1), // UV coordinates flipped for OpenGL
			ImVec2(1, 0)
		);
	}

	ImGui::End();
}

void ChannelSelectorSystem::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);

	// TODO : We only really need to update the rendering when these events occur, so we could set a dirty flag and only re-render on the next update.
	dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(ChannelSelectorSystem::OnMouseScrolled));
	dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(ChannelSelectorSystem::OnMouseButtonPressed));
}

void ChannelSelectorSystem::RenderMenuBar()
{
	// Optional: Add menu bar items
}

bool ChannelSelectorSystem::OnMouseScrolled(const MouseScrolledEvent& event)
{
	if (!viewport_hovered_) return false;

	float zoom_factor = 1.0f + (event.GetYOffset() * 0.1f);
	config_.zoom_level *= zoom_factor;
	config_.zoom_level = std::clamp(config_.zoom_level, 0.1f, 10.0f);

	dirty_ = true;

	return true;
}

bool ChannelSelectorSystem::OnMouseButtonPressed(const MouseButtonPressedEvent& event) {
	if (!viewport_hovered_) return false;

	// Handle middle mouse button for panning
	if (event.GetMouseButton() == Mouse::ButtonMiddle) {
		is_panning_ = true;
		last_mouse_position_ = mouse_position_;
		return true;
	}

	// Handle left click for selection
	if (event.GetMouseButton() == Mouse::ButtonLeft) {
		glm::vec2 world_pos = ScreenToWorld(mouse_position_);

		// Try to select a channel
		auto channel_id = GetChannelAtPosition(world_pos);
		if (channel_id != -1) {
			bool add_to_selection = Input::IsKeyPressed(Key::LeftControl) ||
				Input::IsKeyPressed(Key::RightControl);

			// Check if already selected
			auto it = std::find(selected_channels_.begin(), selected_channels_.end(), channel_id);
			if (it != selected_channels_.end()) {
				// Already selected - toggle off if Ctrl is held
				if (add_to_selection) {
					selected_channels_.erase(it);
				}
			}
			else {
				// Not selected - add it
				if (!add_to_selection) {
					selected_channels_.clear();
				}
				selected_channels_.push_back(channel_id);
			}

			dirty_ = true;
			return true;
		}

		// Could also check sources/detectors here if needed
	}

	return false;
}

void ChannelSelectorSystem::SelectAllChannels() {
	selected_channels_.clear();
	for (auto& [id, channel] : channels_) {
		selected_channels_.push_back(id);
	}
}

void ChannelSelectorSystem::ClearSelection() {
	selected_channels_.clear();
}

// ============================================================================
// RENDERING IMPLEMENTATION
// ============================================================================

void ChannelSelectorSystem::RenderToBuffer()
{
	// Clear buffer
	image_buffer_.Clear(config_.settings.background_color);

	// Draw in order (back to front)
	DrawGrid();

	DrawChannels();
	DrawSources();
	DrawDetectors();
}

void ChannelSelectorSystem::UpdateTexture()
{
	glBindTexture(GL_TEXTURE_2D, texture_id_);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA8,
		image_buffer_.Width,
		image_buffer_.Height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_buffer_.Data.data()
	);
}

void ChannelSelectorSystem::DrawGrid()
{
	// Optional: Draw a subtle grid
	float grid_spacing = config_.settings.world_scale * config_.zoom_level;

	// Draw vertical lines
	for (int x = 0; x < (int)image_buffer_.Width; x += (int)grid_spacing) {
		for (int y = 0; y < (int)image_buffer_.Height; y++) {
			image_buffer_.SetPixel(x, y, config_.settings.grid_color);
		}
	}

	// Draw horizontal lines
	for (int y = 0; y < (int)image_buffer_.Height; y += (int)grid_spacing) {
		for (int x = 0; x < (int)image_buffer_.Width; x++) {
			image_buffer_.SetPixel(x, y, config_.settings.grid_color);
		}
	}
}

void ChannelSelectorSystem::DrawChannels()
{
	for (auto& [id, visual] : channel_visuals_) {
		// Check if selected
		bool is_selected = std::find(selected_channels_.begin(),
			selected_channels_.end(),
			id) != selected_channels_.end();

		uint32_t color = is_selected ? config_.settings.selected_channel_color : config_.settings.channel_color;

		glm::vec2 screen_start = WorldToScreen(visual.Start);
		glm::vec2 screen_end = WorldToScreen(visual.End);

		DrawThickLine(screen_start, screen_end, color, config_.settings.channel_width);
	}
}

void ChannelSelectorSystem::DrawSources()
{
	for (auto& [id, source] : sources_) {
		glm::vec2 world_pos(source.position_2D.x, source.position_2D.y);
		glm::vec2 screen_pos = WorldToScreen(world_pos);

		DrawCircle(screen_pos, config_.settings.source_radius, config_.settings.source_color, true);

		// Optional: Draw border
		DrawCircle(screen_pos, config_.settings.source_radius + 1, 0xFF000000, false);
	}
}

void ChannelSelectorSystem::DrawDetectors()
{
	for (auto& [id, detector] : detectors_) {
		glm::vec2 world_pos(detector.position_2D.x, detector.position_2D.y);
		glm::vec2 screen_pos = WorldToScreen(world_pos);

		DrawCircle(screen_pos, config_.settings.detector_radius, config_.settings.detector_color, true);

		// Optional: Draw border
		DrawCircle(screen_pos, config_.settings.detector_radius + 1, 0xFF000000, false);
	}
}

// ============================================================================
// DRAWING PRIMITIVES
// ============================================================================

void ChannelSelectorSystem::DrawLine(glm::vec2 start, glm::vec2 end, uint32_t color, float width)
{
	// Bresenham's line algorithm with thickness
	int x0 = (int)start.x;
	int y0 = (int)start.y;
	int x1 = (int)end.x;
	int y1 = (int)end.y;

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx - dy;

	while (true) {
		image_buffer_.SetPixel(x0, y0, color);

		if (x0 == x1 && y0 == y1) break;

		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
}

void ChannelSelectorSystem::DrawThickLine(glm::vec2 start, glm::vec2 end, uint32_t color, float width)
{
	// Draw a thick line by drawing multiple offset lines
	glm::vec2 dir = end - start;
	float length = glm::length(dir);
	if (length < 0.001f) return;

	dir /= length;
	glm::vec2 perpendicular(-dir.y, dir.x);

	int half_width = (int)(width / 2.0f);
	for (int i = -half_width; i <= half_width; i++) {
		glm::vec2 offset = perpendicular * (float)i;
		DrawLine(start + offset, end + offset, color, 1.0f);
	}
}

void ChannelSelectorSystem::DrawCircle(glm::vec2 center, float radius, uint32_t color, bool filled)
{
	int cx = (int)center.x;
	int cy = (int)center.y;
	int r = (int)radius;

	if (filled) {
		// Filled circle using midpoint algorithm
		for (int y = -r; y <= r; y++) {
			for (int x = -r; x <= r; x++) {
				if (x * x + y * y <= r * r) {
					image_buffer_.SetPixel(cx + x, cy + y, color);
				}
			}
		}
	}
	else {
		// Circle outline using midpoint algorithm
		int x = r;
		int y = 0;
		int err = 0;

		while (x >= y) {
			image_buffer_.SetPixel(cx + x, cy + y, color);
			image_buffer_.SetPixel(cx + y, cy + x, color);
			image_buffer_.SetPixel(cx - y, cy + x, color);
			image_buffer_.SetPixel(cx - x, cy + y, color);
			image_buffer_.SetPixel(cx - x, cy - y, color);
			image_buffer_.SetPixel(cx - y, cy - x, color);
			image_buffer_.SetPixel(cx + y, cy - x, color);
			image_buffer_.SetPixel(cx + x, cy - y, color);

			if (err <= 0) {
				y += 1;
				err += 2 * y + 1;
			}
			if (err > 0) {
				x -= 1;
				err -= 2 * x + 1;
			}
		}
	}
}

// ============================================================================
// COORDINATE TRANSFORMS
// ============================================================================
glm::vec2 ChannelSelectorSystem::WorldToScreen(const glm::vec2& worldPos) const
{
	// Apply zoom and pan
	glm::vec2 viewPos = (worldPos - config_.view_center) * config_.settings.world_scale * config_.zoom_level;

	// Center in viewport
	glm::vec2 screenPos = viewPos + glm::vec2(image_buffer_.Width / 2.0f, image_buffer_.Height / 2.0f);

	// NO Y-FLIP HERE - ImGui UV coordinates already handle it
	// The UV coords (0,1) to (1,0) in ImGui::Image flip Y for us

	return screenPos + config_.pan_offset;
}

glm::vec2 ChannelSelectorSystem::ScreenToWorld(const glm::vec2& screenPos) const
{
	// NO Y-FLIP HERE - mouse position is already in screen space matching the image

	// Remove pan offset
	glm::vec2 adjustedScreen = screenPos - config_.pan_offset;

	// Remove viewport centering
	glm::vec2 viewPos = adjustedScreen - glm::vec2(image_buffer_.Width / 2.0f, image_buffer_.Height / 2.0f);

	// Remove zoom and scale
	glm::vec2 worldPos = (viewPos / (config_.settings.world_scale * config_.zoom_level)) + config_.view_center;

	return worldPos;
}

// ============================================================================
// HIT TESTING
// ============================================================================

NIRS::Probe::ChannelID ChannelSelectorSystem::GetChannelAtPosition(const glm::vec2& world_pos)
{
	float threshold = 0.5f / config_.zoom_level; // Adjust hit detection based on zoom

	for (auto& [id, visual] : channel_visuals_) {
		// Calculate distance from point to line segment
		glm::vec2 line = visual.End - visual.Start;
		float line_length = glm::length(line);
		if (line_length < 0.001f) continue;

		glm::vec2 line_dir = line / line_length;
		glm::vec2 point_vec = world_pos - visual.Start;

		// Project point onto line
		float t = glm::dot(point_vec, line_dir);
		t = std::clamp(t, 0.0f, line_length);

		glm::vec2 closest_point = visual.Start + line_dir * t;
		float distance = glm::length(world_pos - closest_point);

		if (distance < threshold) {
			return id;
		}
	}

	return -1; // No channel found
}

NIRS::Probe::OptodeID ChannelSelectorSystem::GetSourceAtPosition(const glm::vec2& world_pos)
{
	float threshold = (config_.settings.source_radius / config_.settings.world_scale) / config_.zoom_level;

	for (auto& [id, source] : sources_) {
		glm::vec2 source_pos(source.position_2D.x, source.position_2D.y);
		float distance = glm::length(world_pos - source_pos);

		if (distance < threshold) {
			return id;
		}
	}

	return -1;
}

NIRS::Probe::OptodeID ChannelSelectorSystem::GetDetectorAtPosition(const glm::vec2& world_pos)
{
	float threshold = (config_.settings.detector_radius / config_.settings.world_scale) / config_.zoom_level;

	for (auto& [id, detector] : detectors_) {
		glm::vec2 detector_pos(detector.position_2D.x, detector.position_2D.y);
		float distance = glm::length(world_pos - detector_pos);

		if (distance < threshold) {
			return id;
		}
	}

	return -1;
}

// ============================================================================
// VIEW MANAGEMENT
// ============================================================================

void ChannelSelectorSystem::FitViewToData()
{
	if (sources_.empty() && detectors_.empty()) return;

	glm::vec2 min, max;
	CalculateWorldBounds(min, max);

	// Calculate center
	config_.view_center = (min + max) * 0.5f;

	// Calculate required zoom to fit
	glm::vec2 size = max - min;
	float viewport_aspect = viewport_size_.x / viewport_size_.y;
	float data_aspect = size.x / size.y;

	if (data_aspect > viewport_aspect) {
		// Width-limited
		config_.zoom_level = viewport_size_.x / (size.x * config_.settings.world_scale * 1.2f);
	}
	else {
		// Height-limited
		config_.zoom_level = viewport_size_.y / (size.y * config_.settings.world_scale * 1.2f);
	}

	config_.zoom_level = std::clamp(config_.zoom_level, 0.1f, 10.0f);
	config_.pan_offset = { 0.0f, 0.0f };
}

void ChannelSelectorSystem::CalculateWorldBounds(glm::vec2& min, glm::vec2& max)
{
	bool first = true;

	for (auto& [id, source] : sources_) {
		glm::vec2 pos(source.position_2D.x, source.position_2D.y);
		if (first) {
			min = max = pos;
			first = false;
		}
		else {
			min = glm::min(min, pos);
			max = glm::max(max, pos);
		}
	}

	for (auto& [id, detector] : detectors_) {
		glm::vec2 pos(detector.position_2D.x, detector.position_2D.y);
		min = glm::min(min, pos);
		max = glm::max(max, pos);
	}

	// Add padding
	glm::vec2 padding = (max - min) * 0.1f;
	min -= padding;
	max += padding;
}
