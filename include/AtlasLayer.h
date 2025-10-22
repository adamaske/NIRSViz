#pragma once

#include "Core/Layer.h"
#include "Renderer/Renderer.h"

#include "Renderer/Mesh.h"
#include "Renderer/Shader.h"
#include "Renderer/RoamCamera.h"
#include "Renderer/OrbitCamera.h"





class AtlasLayer : public Layer {
public:
	AtlasLayer();
	~AtlasLayer();


	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(float dt) override;
	void OnRender() override;

	void OnImGuiRender()override;

	void OnEvent(Event& event) override;

	void RenderMenuBar() override;
	void RenderCameraSettings(bool standalone);
	void RenderMainViewport();

private:
	ViewID m_ViewTargetID = 2; // Passed to renderer to specify this viewport

	Ref<Framebuffer> m_EditorFramebuffer = nullptr;

	Ref<Mesh> m_HeadMesh = nullptr;
	bool m_DrawHead = true;
	float m_HeadOpacity = 0.5f;

	Ref<Mesh> m_CortexMesh = nullptr;
	bool m_DrawCortex = true;

	Ref<Shader> m_PhongShader = nullptr;


	UniformData m_LightPosUniform;
	UniformData m_ObjectColorUniform;
	UniformData m_OpacityUniform;
};