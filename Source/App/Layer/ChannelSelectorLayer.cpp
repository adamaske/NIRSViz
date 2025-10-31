#include "pch.h"
#include "App/Layer/ChannelSelectorLayer.h"

#include <imgui.h>
#include "Renderer/ViewportManager.h"

#include "Core/Input.h"
#include "Core/AssetManager.h"	

#include "Events/MouseCodes.h"
#include "Events/KeyCodes.h"
#include "Events/EventBus.h"

#define BACKGROUND_ID 0
#define SOURCE_OFFSET 512
#define DETECTOR_OFFSET 1024
#define CHANNEL_OFFSET 2048

ChannelSelectorLayer::ChannelSelectorLayer(const EntityID& settingsID) : Layer(settingsID)
{
}
ChannelSelectorLayer::~ChannelSelectorLayer()
{
}

void ChannelSelectorLayer::OnAttach()
{
	FramebufferSpecification fbSpec; // Init a framebuffer to show the probe
	// Double Color
	fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
	fbSpec.Width = 800;
	fbSpec.Height = 600;
	m_Framebuffer = CreateRef<Framebuffer>(fbSpec);
	
	m_OrthoCamera = CreateRef<OrthogonalCamera>(glm::vec3{ 0, 0, -10 }, glm::vec3{ 0, 0, 1 });
	m_OrthoCamera->SetZoomLevel(5.0f);
	ViewportManager::RegisterViewport({ "ChannelSelectorViewport", CHANNEL_SELECTOR, m_OrthoCamera, m_Framebuffer });

	m_TextureShader = CreateRef<Shader>("C:/dev/NIRSViz/Assets/Shaders/Texture.vert",
										"C:/dev/NIRSViz/Assets/Shaders/Texture.frag");

	m_FlatColorShader = CreateRef<Shader>("C:/dev/NIRSViz/Assets/Shaders/FlatColor.vert",
										"C:/dev/NIRSViz/Assets/Shaders/FlatColor.frag");

	m_SourceTexture = CreateRef<Texture>("C:/dev/NIRSViz/Assets/Textures/source.png");
	m_DetectorTexture = CreateRef<Texture>("C:/dev/NIRSViz/Assets/Textures/detector.png");
	m_ChannelTexture = CreateRef<Texture>("C:/dev/NIRSViz/Assets/Textures/channel.png");
	m_BackgroundTexture = CreateRef<Texture>("C:/dev/NIRSViz/Assets/Textures/background.png");

	m_QuadMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/plane.obj");
	m_PlateMesh = CreateRef<Mesh>("C:/dev/NIRSViz/Assets/Models/plate.obj");
	EventBus::Instance().Subscribe<OnSNIRFLoaded>([this](const OnSNIRFLoaded& e){
		this->HandleSNIRFLoaded();
	});
}

void ChannelSelectorLayer::OnDetach()
{
}

void ChannelSelectorLayer::OnUpdate(float dt)
{
	auto camera = ViewportManager::GetViewport("ChannelSelectorViewport").CameraPtr;
	camera->UpdateProjectionMatrix();
	camera->UpdateViewMatrix();

	DrawBackground();
	DrawSourcesAndDetectors();
	DrawChannels();
}

void ChannelSelectorLayer::OnRender()
{
}

void ChannelSelectorLayer::OnImGuiRender()
{
	ImGui::Begin("Channel Selector Settings");
	ImGui::SliderFloat("Grid Scale", &m_GridScale, 0.01f, 10.0f);
	ImGui::SliderFloat("Plate Size", &m_PlateSize, 0.1f, 10.0f);
	ImGui::SliderFloat("Channel Width", &m_ChannelWidth, 0.01f, 1.0f);
	ImGui::SeparatorText("Camera Settings");
	ImGui::Text("Use Mouse Scroll to Zoom In/Out");
	ImGui::Text("Use Right Click to Select Channel");
	m_OrthoCamera->OnImGuiRender(false);

	ImGui::End();


	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
	ImGui::Begin("Channel Selector");

	if (ImGui::Button("Select All")) {
		m_SelectedChannels.clear();
		for (auto& [ID, channel] : m_Channels) {
			m_SelectedChannels.push_back(ID);
		}
		EventBus::Instance().Publish<OnChannelsSelected>(OnChannelsSelected{ m_SelectedChannels });
	}
	ImGui::SameLine();
	if(ImGui::Button("Select None")) {
		m_SelectedChannels.clear();
		EventBus::Instance().Publish<OnChannelsSelected>(OnChannelsSelected{ m_SelectedChannels });
	}

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();
	m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	m_ViewportFocused = ImGui::IsWindowFocused();
	m_ViewportHovered = ImGui::IsWindowHovered();

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		m_MousePosition = { mx, my };
	}

	if (m_Framebuffer->GetSpecification().Width != (uint32_t)viewportPanelSize.x ||
		m_Framebuffer->GetSpecification().Height != (uint32_t)viewportPanelSize.y)
	{
		if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
		{
			m_Framebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
		}
	}

	ViewportManager::GetViewport("ChannelSelectorViewport").CameraPtr->SetViewportSize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);

	uint32_t texture_id = m_Framebuffer->GetColorAttachmentRendererID();
	ImGui::Image((void*)(intptr_t)texture_id, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();

}

void ChannelSelectorLayer::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(ChannelSelectorLayer::OnMouseScrolled));
	dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(ChannelSelectorLayer::OnMouseButtonPressed));
}

void ChannelSelectorLayer::RenderMenuBar()
{
}

bool ChannelSelectorLayer::OnMouseScrolled(const MouseScrolledEvent& event)
{
	if (!m_ViewportHovered) return false;
	auto zoomOffset = event.GetYOffset();
	auto newZoom = m_OrthoCamera->GetZoomLevel() + (zoomOffset * 0.03f);
	m_OrthoCamera->SetZoomLevel(newZoom );
	return false;
}

bool ChannelSelectorLayer::OnMouseButtonPressed(const MouseButtonPressedEvent& event)
{
	if (!m_ViewportHovered) return false;
	
	m_Framebuffer->Bind();

	auto mousePos = Input::GetMousePosition();
	int id = m_Framebuffer->ReadPixel(1, (int)m_MousePosition.x, (int)(m_MousePosition.y));
	m_Framebuffer->Unbind();

	bool add = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
	if (id == 0) {
		// Pressed background

	}
	else if (id - CHANNEL_OFFSET >= 0) {
		auto channelID = id - CHANNEL_OFFSET;
		NVIZ_INFO("Pressed Channel ID: {}", channelID);
		
		// Is this channel already selected?
		for (size_t i = 0; i < m_SelectedChannels.size(); i++)
		{
			if(id == m_SelectedChannels[i]) {
				return true;
			}
		}

		if (!add) {
			m_SelectedChannels.clear();
		}

		m_SelectedChannels.push_back(channelID);
		EventBus::Instance().Publish<OnChannelsSelected>(OnChannelsSelected{ m_SelectedChannels });
		return true;
		// Pressed Channel
	}else if (id - DETECTOR_OFFSET >= 0) {
		auto detectorID = id - DETECTOR_OFFSET;
		NVIZ_INFO("Pressed Detector ID: {}", detectorID);
		// Pressed Detector

	}
	else if (id - SOURCE_OFFSET >= 0) {
		auto sourceID = id - SOURCE_OFFSET;
		NVIZ_INFO("Pressed Source ID: {}", sourceID);
		// Pressed Source
	}


	return true; // Capture the event
}

void ChannelSelectorLayer::HandleSNIRFLoaded()
{
	auto snirf = AssetManager::Get<SNIRF>("SNIRF");
	m_ChannelVisuals.clear(); 
	m_SelectedChannels.clear();
	m_Channels = snirf->GetChannelMap();
	m_Sources = snirf->GetSource2DMap();
	m_Detectors = snirf->GetDetector2DMap();

	for (auto& [ID, channel] : m_Channels) {
		auto source = m_Sources[channel.SourceID - 1];
		auto detector = m_Detectors[channel.DetectorID - 1];

		auto startPos = source.Position;
		auto endPos = detector.Position;

		m_ChannelVisuals[ID] = Channel2DVisual{
			glm::vec3(-startPos.x, startPos.y, 0.0f),
			glm::vec3(-endPos.x, endPos.y, 0.0f),
			ID
		};

		m_SelectedChannels.push_back(ID); // Select all channels by default
		NVIZ_INFO("Channel: {}, Source {} Detector {} Wavelength {}", ID, channel.SourceID, channel.DetectorID, 0);
	}
}

void ChannelSelectorLayer::DrawBackground()
{
	RenderCommand cmd;
	cmd.ShaderPtr = m_TextureShader.get();
	cmd.ViewTargetID = CHANNEL_SELECTOR;
	cmd.Transform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 1)), glm::vec3(100));
	cmd.VAOPtr = m_QuadMesh->GetVAO().get();
	cmd.Mode = DRAW_ELEMENTS;

	TextureBinding texBind;
	texBind.Slot = 3;
	texBind.TexturePtr = m_BackgroundTexture.get();
	cmd.TextureBindings = { texBind };

	UniformData texUnifrom;
	texUnifrom.Type = UniformDataType::SAMPLER2D;
	texUnifrom.Name = "u_Texture";
	texUnifrom.Data.i1 = 3;
	UniformData id;
	id.Name = "u_ID";
	id.Type = UniformDataType::INT1;
	id.Data.i1 = BACKGROUND_ID;
	cmd.UniformCommands = { texUnifrom, id };

	Renderer::Submit(cmd);
}

void ChannelSelectorLayer::DrawSourcesAndDetectors()
{
	glm::vec3 centerSum(0.0f);
	// 1. Sum up all source positions
	for (auto& [ID, source] : m_Sources) {
		centerSum += glm::vec3(source.Position.x, source.Position.y, 0);
	}
	glm::vec3 centerPos = centerSum / (float)m_Sources.size(); 
	centerPos = glm::vec3(-centerPos.x, centerPos.y, 0.0f) * (m_GridScale * 0.1f);

	m_OrthoCamera->SetPosition(glm::vec3(centerPos.x, centerPos.y, -10.0f));

	for (auto& [ID, source] : m_Sources) {
		RenderCommand cmd;
		cmd.ShaderPtr = m_TextureShader.get();
		cmd.ViewTargetID = CHANNEL_SELECTOR;
		auto pos = glm::vec3(source.Position.x, source.Position.y, 0.0f) * (m_GridScale * 0.1f);
		cmd.Transform = glm::scale(glm::translate(glm::mat4(1.0f), pos), glm::vec3(m_PlateSize));
		cmd.VAOPtr = m_PlateMesh->GetVAO().get();
		cmd.Mode = DRAW_ELEMENTS;

		TextureBinding texBind;
		texBind.Slot = 0;
		texBind.TexturePtr = m_SourceTexture.get();
		cmd.TextureBindings = { texBind };

		UniformData texUnifrom;
		texUnifrom.Type = UniformDataType::SAMPLER2D;
		texUnifrom.Name = "u_Texture";
		texUnifrom.Data.i1 = 0;
		UniformData id;
		id.Name = "u_ID";
		id.Type = UniformDataType::INT1;
		id.Data.i1 = source.ID + SOURCE_OFFSET;
		cmd.UniformCommands = { texUnifrom, id };

		Renderer::Submit(cmd);
	}

	// Draw Detectors
	for (auto& [ID, detector] : m_Detectors) {
		RenderCommand cmd;
		cmd.ShaderPtr = m_TextureShader.get();
		cmd.ViewTargetID = CHANNEL_SELECTOR;
		auto pos = glm::vec3(detector.Position.x, detector.Position.y, 0.0f) * (m_GridScale * 0.1f);
		cmd.Transform = glm::scale(glm::translate(glm::mat4(1.0f), pos), glm::vec3(m_PlateSize));
		cmd.VAOPtr = m_PlateMesh->GetVAO().get();
		cmd.Mode = DRAW_ELEMENTS;

		TextureBinding texBind;
		texBind.Slot = 1;
		texBind.TexturePtr = m_DetectorTexture.get();
		cmd.TextureBindings = { texBind };

		UniformData texUnifrom;
		texUnifrom.Type = UniformDataType::SAMPLER2D;
		texUnifrom.Name = "u_Texture";
		texUnifrom.Data.i1 = 1;
		UniformData id;
		id.Name = "u_ID";
		id.Type = UniformDataType::INT1;
		id.Data.i1 = detector.ID + DETECTOR_OFFSET;
		cmd.UniformCommands = { texUnifrom, id };

		Renderer::Submit(cmd);
	}

	// Position the camera above the center of the sources and detectors
	// Find the center
	// m_OrthoCamera->SetPosition(glm::vec3(centerPos.x, centerPos.y, -10.0f));
}

void ChannelSelectorLayer::DrawChannels()
{

	for (auto& [ID, channel] : m_Channels) {
		auto visual = m_ChannelVisuals[ID];

		glm::vec3 startPos = visual.Start * (m_GridScale * 0.1f);
		glm::vec3 endPos = visual.End * (m_GridScale * 0.1f);
		glm::vec3 centerPos = (startPos + endPos) / 2.0f;
		glm::vec3 dir = endPos - startPos;
		float length = glm::length(dir);
		float angle = atan2(dir.y, dir.x);

		RenderCommand cmd;
		cmd.ShaderPtr = m_TextureShader.get();
		cmd.ViewTargetID = CHANNEL_SELECTOR;
		cmd.Transform = glm::translate(glm::mat4(1.0f), centerPos) *
						glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1)) *
						glm::scale(glm::mat4(1.0f), glm::vec3(length, m_ChannelWidth, 1.0f));

		cmd.VAOPtr = m_QuadMesh->GetVAO().get();
		cmd.Mode = DRAW_ELEMENTS;

		TextureBinding texBind;
		texBind.Slot = 2;
		texBind.TexturePtr = m_ChannelTexture.get();
		cmd.TextureBindings = { texBind };
		UniformData texUnifrom;
		texUnifrom.Type = UniformDataType::SAMPLER2D;
		texUnifrom.Name = "u_Texture";
		texUnifrom.Data.i1 = 2;
		UniformData id;
		id.Name = "u_ID";
		id.Type = UniformDataType::INT1;
		id.Data.i1 = ID + CHANNEL_OFFSET;
		cmd.UniformCommands = { texUnifrom, id };
		Renderer::Submit(cmd);
	}

}
