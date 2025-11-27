#pragma once
#include "Core/Base.h"
#include "Renderer/Buffer/VertexArray.h"
#include "Renderer/Buffer/Framebuffer.h"
#include "Renderer/Camera/Camera.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderable/Texture.h"
using ViewID = uint32_t;

struct RenderView {
	Ref<Camera> Camera = nullptr;
	Ref<Framebuffer> TargetFBO = nullptr;
	// You could add global shader uniform data here if needed (e.g., global light direction)
};

enum DrawMode {
	DRAW_ELEMENTS = 0,
	DRAW_LINES = 1,
	DRAW_ARRAYS = 2,
	DRAW_POINTS = 3
};

enum class UniformDataType {
	FLOAT1, FLOAT3, FLOAT2, FLOAT4, MAT4, INT1, BOOL1, SAMPLER1D, SAMPLER2D
};

struct UniformData {
	UniformDataType Type;
	std::string Name; // Name of the uniform in the shader

	union Value {
		float f1;
		glm::vec2 f2;
		glm::vec3 f3;
		glm::vec4 f4;
		glm::mat4 m4;
		int i1;
		bool b1;
	} Data;
};
struct TextureBinding
{
	Texture* TexturePtr = nullptr;
	uint32_t Slot = 0; // The texture unit slot to bind to (GL_TEXTURE0 + Slot)
};

// Renderer:: has a bunch of static functions for API calls
struct RendererAPICall {
	std::function<void()> call;// Here the user puts in the static function or lambda or sometighn
};

struct RenderCommand {
	Shader* ShaderPtr = nullptr;
	VertexArray* VAOPtr = nullptr;
	glm::mat4 Transform = glm::mat4(1.0f);

	// NEW: Which target this command should draw into
	ViewID ViewTargetID = 0;
	DrawMode Mode = DRAW_ELEMENTS;

	std::vector<TextureBinding> TextureBindings = {};
	std::vector<UniformData> UniformCommands = {};
	std::vector<RendererAPICall> APICalls = {};
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
	static void DrawArrays(const VertexArray* vertexArray, uint32_t vertexCount);
	static void DrawPoints(const VertexArray* vertexArray, uint32_t vertexCount);

	static void OnWindowResize(uint32_t width, uint32_t height);
	static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
	static void SetClearColor(const glm::vec4& color);
	static void Clear();
	static void SetLineWidth(float width);
	static void SetPointSize(float size);
	static void EnableDepthTest(bool enable);

	static void SetDepthMask(bool write);
private:
	static Scope<RendererData> s_Data;

	static Ref<Framebuffer> m_CurrentBoundFBO;
	static Ref<Camera> m_CurrentBoundCamera;
};