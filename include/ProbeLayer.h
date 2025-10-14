#pragma once

#include "Core/Layer.h"

#include "Renderer/Shader.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/RoamCamera.h"
#include "Renderer/OrbitCamera.h"
#include "Renderer/VertexArray.h"
#include "Mesh.h"

class ProbeLayer : public Layer {
public:
	ProbeLayer();
	~ProbeLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(float dt) override;
	virtual void OnRender() override;

	virtual void OnImGuiRender()override;

	virtual void OnEvent(Event& event) override;

	void RenderProbeViewport();
	void RenderProbeInformation();
	void LoadProbeButton();
	void LoadProbeFile(const std::string& filepath);


private:
	std::string m_CurrentFilepath = "";
	bool m_ProbeLoaded = false;
	bool m_DrawProbe = true;
	bool m_UseRoamCamera = true;
	
	uint32_t m_ViewTargetID = 1; // Passed to renderer to specify this viewport

	Ref<Framebuffer> m_Framebuffer = nullptr;
	Ref<RoamCamera> m_RoamCamera = nullptr;
	Ref<OrbitCamera> m_OrbitCamera = nullptr;

	Ref<Camera> GetActiveCamera() {
		return m_UseRoamCamera ? m_RoamCamera : (Ref<Camera>)m_OrbitCamera;
	}

	Ref<Shader> m_FlatColorShader = nullptr;
	Ref<VertexArray> m_ProbeVAO = nullptr;
	Ref<Mesh> m_ProbeMesh = nullptr;
};