#pragma once

#include "Core/Base.h"
#include "Systems/System.h"

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderable/Mesh.h"
#include "Renderer/Renderable/Texture.h"

#include "Events/MouseEvent.h"
#include "Renderer/Buffer/Framebuffer.h"
#include "Renderer/Camera/OrthogonalCamera.h"

#include <set>

class ISelectedChannelsProvider {
public:
	virtual const std::vector<NIRS::Probe::ChannelID>& GetSelectedChannels() = 0;
};

struct Channel2DVisual {
	glm::vec3 Start;
	glm::vec3 End;

	NIRS::Probe::ChannelID ChannelID;
};

class ChannelSelectorSystem : public System, public ISelectedChannelsProvider
{
public:
	ChannelSelectorSystem();
	~ChannelSelectorSystem();

	const std::vector<NIRS::Probe::ChannelID>& GetSelectedChannels() override {
		return m_SelectedChannels;
	};

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(DeltaTime dt) override;

	void OnGUIRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;

	bool OnMouseScrolled(const MouseScrolledEvent& event);
	bool OnMouseButtonPressed(const MouseButtonPressedEvent& event);

	void HandleSNIRFLoaded();

	void SelectAllChannels();
	void ClearSelection();
private:
	Ref<Framebuffer> m_Framebuffer = nullptr;
	Ref<OrthogonalCamera> m_OrthoCamera = nullptr;

	Ref<Shader> m_TextureShader = nullptr;
	Ref<Shader> m_FlatColorShader = nullptr;

	Ref<Mesh> m_QuadMesh = nullptr;
	Ref<Mesh> m_PlateMesh = nullptr;

	Ref<Texture> m_SourceTexture = nullptr;
	Ref<Texture> m_DetectorTexture = nullptr;
	Ref<Texture> m_ChannelTexture = nullptr;
	Ref<Texture> m_BackgroundTexture = nullptr;

	float m_GridScale = 1.f;
	float m_PlateSize = 1.f;
	float m_ChannelWidth = 0.2f;

	// --- Channel Data ---
	glm::vec2 m_MousePosition;
	glm::vec2 previous_mouse_position_;

	std::map<NIRS::Probe::ChannelID, Channel2DVisual> m_ChannelVisuals;
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::Channel> m_Channels;

	std::vector<NIRS::Probe::ChannelID> m_SelectedChannels = {};

	std::map<NIRS::Probe::OptodeID, NIRS::Probe::Optode> sources_;
	std::map<NIRS::Probe::OptodeID, NIRS::Probe::Optode> detectors_;

	// --- Viewport Settings

	bool m_ViewportHovered = false;
	bool m_ViewportFocused = false;
	glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
	glm::vec2 m_ViewportBounds[2];

	std::vector<RenderCommand> m_BackgroundRenderCommands = {};
	std::vector<RenderCommand> m_SourceRenderCommands = {};
	std::vector<RenderCommand> m_DetectorRenderCommands = {};
	std::vector<RenderCommand> m_ChannelRenderCommands = {};

	void GenerateSourceRenderCommands();
	void GenerateDetectorRenderCommands();
	void GenerateChannelRenderCommands();
	void GenerateBackgroundRenderCommands();

	void DrawBackground();
	void DrawSourcesAndDetectors();
	void DrawChannels();

	ViewportType viewport_type_ = ViewportType::ChannelSelector;
};