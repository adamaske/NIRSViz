#pragma once

#include "Core/Base.h"
#include "Core/Layer.h"

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderable/Mesh.h"
#include "Renderer/Renderable/Texture.h"

#include "Events/MouseEvent.h"
#include "Renderer/Buffer/Framebuffer.h"
#include "Renderer/Camera/OrthogonalCamera.h"

struct Channel2DVisual {
	glm::vec3 Start;
	glm::vec3 End;

	NIRS::ChannelID ChannelID;
};

class ChannelSelectorLayer : public Layer 
{
public:
	ChannelSelectorLayer(const EntityID& settingsID);
	~ChannelSelectorLayer();

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;

	bool OnMouseScrolled(const MouseScrolledEvent& event);
	bool OnMouseButtonPressed(const MouseButtonPressedEvent& event);

	void HandleSNIRFLoaded();

	void DrawBackground();
	void DrawSourcesAndDetectors();
	void DrawChannels();
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
	std::map<NIRS::ChannelID, Channel2DVisual> m_ChannelVisuals;
	std::map<NIRS::ChannelID, NIRS::Channel> m_Channels;

	std::vector<NIRS::ChannelID> m_SelectedChannels = {};

	std::map<NIRS::ProbeID, NIRS::Probe2D> m_Sources;
	std::map<NIRS::ProbeID, NIRS::Probe2D> m_Detectors;

	// --- Viewport Settings
	bool m_ViewportHovered = false;
	bool m_ViewportFocused = false;
	glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
	glm::vec2 m_ViewportBounds[2];


};