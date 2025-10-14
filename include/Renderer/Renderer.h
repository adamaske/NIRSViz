#pragma once
#include "Core/Base.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Camera.h"
#include "Renderer/Shader.h"

using ViewID = uint32_t;

struct RenderView {
	Ref<Camera> Camera = nullptr;
	Ref<Framebuffer> TargetFBO = nullptr;
	// You could add global shader uniform data here if needed (e.g., global light direction)
};

enum DrawMode {
	DRAW_ELEMENTS = 0,
	DRAW_LINES = 1,
};

struct RenderCommand {
	Shader* ShaderPtr = nullptr;
	VertexArray* VAOPtr = nullptr;
	glm::mat4 Transform = glm::mat4(1.0f);

	// NEW: Which target this command should draw into
	ViewID ViewTargetID = 0;
	DrawMode Mode = DRAW_ELEMENTS;
};

struct RendererData
{
	// The queue where systems submit their draw requests
	std::vector<RenderCommand> CommandQueue;
	std::unordered_map<ViewID, RenderView> ActiveViews;
};

class Renderer {
public:
	static void Init();
	static void Shutdown();

	static void BeginScene();
	static void EndScene();

	static void RegisterView(ViewID id, Ref<Camera> camera, Ref<Framebuffer> framebuffer) {
		s_Data->ActiveViews[id] = { camera, framebuffer };
	}

	static void ExecuteQueue();

	static void Submit(const RenderCommand& command);
	static void Submit(Shader& shader, VertexArray& va, const glm::mat4& transform, ViewID viewId, DrawMode mode);

	static void DrawIndexed(const VertexArray* vertexArray, uint32_t indexCount = 0);
	static void DrawLines(const VertexArray* vertexArray, uint32_t vertexCount);

	static void OnWindowResize(uint32_t width, uint32_t height);
	static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
	static void SetClearColor(const glm::vec4& color);
	static void Clear();
	static void SetLineWidth(float width);

private:
	static Scope<RendererData> s_Data;
};