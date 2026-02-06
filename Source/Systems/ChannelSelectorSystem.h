#pragma once
#include "NIRS/ChannelSelector.h"

#include "Core/Base.h"
#include "Systems/System.h"

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"
#include "NIRS/SNIRFProvider.h"

#include "Events/MouseEvent.h"

#include <vector>
#include <map>


struct ChannelSelectorSettings {
	float source_radius = 10.0f;
	float detector_radius = 8.0f;
	float channel_width = 8.0f;
	float world_scale = 20.0f; // Pixels per world unit
	uint32_t background_color = 0xFF202020; // Dark gray
	uint32_t source_color = 0xFF3333FF;     // Red
	uint32_t detector_color = 0xFF0000FF;   // Blue
	uint32_t channel_color = 0xFFAAAAAA;    // Gray
	uint32_t selected_channel_color = 0xFF40FFFF; // Yellow
	uint32_t grid_color = 0xFF303030;       // Darker gray
};

struct ChannelSelectorConfig {
	ChannelSelectorSettings settings;

	// Runtime state (zoom, pan, etc.)
	glm::vec2 pan_offset = { 0.0f, 0.0f };
	float zoom_level = 1.0f;
	glm::vec2 view_center = { 0.0f, 0.0f };
};


struct PixelBuffer {
	std::vector<uint32_t> Data; // RGBA8 format
	uint32_t Width;
	uint32_t Height;

	PixelBuffer(uint32_t w, uint32_t h)
		: Width(w), Height(h), Data(w* h, 0xFF202020) {
	} // Dark gray default

	void Clear(uint32_t color = 0xFF202020) {
		std::fill(Data.begin(), Data.end(), color);
	}

	void SetPixel(int x, int y, uint32_t color) {
		if (x >= 0 && x < (int)Width && y >= 0 && y < (int)Height) {
			Data[y * Width + x] = color;
		}
	}

	void Resize(uint32_t w, uint32_t h) {
		Width = w;
		Height = h;
		Data.resize(w * h, 0xFF202020);
	}
};

class ChannelSelectorSystem : public System, public ISelectedChannelsProvider
{
public:
	ChannelSelectorSystem(ISNIRFProvider& snirf_provider) : snirf_provider_(snirf_provider) {};
	~ChannelSelectorSystem() = default;


	const std::vector<NIRS::Probe::ChannelID>& GetSelectedChannels() override {
		return selected_channels_;
	}

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(DeltaTime dt) override;
	void OnGUIRender() override;
	void OnEvent(Event& event) override;
	void RenderMenuBar() override;

	bool OnMouseScrolled(const MouseScrolledEvent& event);
	bool OnMouseButtonPressed(const MouseButtonPressedEvent& event);

	void HandleSNIRFLoaded();
	void SelectAllChannels();
	void ClearSelection();


private:
	bool dirty_;
	ISNIRFProvider& snirf_provider_;
	// Configuration (settings + runtime state)
	ChannelSelectorConfig config_;

	// Image buffer and texture
	PixelBuffer image_buffer_{ 800, 600 };
	uint32_t texture_id_ = 0; // OpenGL texture ID

	// Channel Data
	std::map<NIRS::Probe::ChannelID, Channel2DVisual> channel_visuals_;
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::Channel> channels_;
	std::vector<NIRS::Probe::ChannelID> selected_channels_;

	std::map<NIRS::Probe::OptodeID, NIRS::Probe::Optode> sources_;
	std::map<NIRS::Probe::OptodeID, NIRS::Probe::Optode> detectors_;

	// Viewport state
	bool viewport_hovered_ = false;
	bool viewport_focused_ = false;
	glm::vec2 viewport_size_ = { 0.0f, 0.0f };
	glm::vec2 mouse_position_ = { 0.0f, 0.0f };
	glm::vec2 last_mouse_position_ = { 0.0f, 0.0f };
	bool is_panning_ = false;

	// Drawing functions
	void RenderToBuffer();
	void UpdateTexture();
	void DrawGrid();
	void DrawChannels();
	void DrawSources();
	void DrawDetectors();

	// Drawing primitives
	void DrawLine(glm::vec2 start, glm::vec2 end, uint32_t color, float width);
	void DrawCircle(glm::vec2 center, float radius, uint32_t color, bool filled = true);
	void DrawThickLine(glm::vec2 start, glm::vec2 end, uint32_t color, float width);

	// Coordinate transforms
	glm::vec2 WorldToScreen(const glm::vec2& worldPos) const;
	glm::vec2 ScreenToWorld(const glm::vec2& screenPos) const;

	// Hit testing
	NIRS::Probe::ChannelID GetChannelAtPosition(const glm::vec2& worldPos);
	NIRS::Probe::OptodeID GetSourceAtPosition(const glm::vec2& worldPos);
	NIRS::Probe::OptodeID GetDetectorAtPosition(const glm::vec2& worldPos);

	// Auto-fit view
	void FitViewToData();
	void CalculateWorldBounds(glm::vec2& min, glm::vec2& max);

};
