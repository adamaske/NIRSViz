#pragma once
#include "Core/Base.h"
#include "Renderer/Buffer/VertexArray.h"
#include "Renderer/Buffer/Framebuffer.h"
#include "Renderer/Camera/Camera.h"
#include "Renderer/Renderable/Shader.h"
#include "Renderer/Renderable/Texture.h"

using ViewID = uint32_t;


enum class ViewportType : uint32_t
{
	NONE = 0,
	MainViewport = 1,
	AnatomyViewport = 2,
	AtlasViewport = 3,
	ProbeEditor = 4,
	ChannelSelector = 5,
	VoxelViewport = 6,
	MRIViewport = 7
};

// Now this is a little more complicated than it needs to be, we can just use a enum instead of having these ID's and such

constexpr uint32_t MAIN_VIEWPORT = static_cast<uint32_t>(ViewportType::MainViewport);
constexpr uint32_t ANATOMY_VIEWPORT = static_cast<uint32_t>(ViewportType::AnatomyViewport);
constexpr uint32_t ATLAS_VIEWPORT = static_cast<uint32_t>(ViewportType::AtlasViewport);
constexpr uint32_t PROBE_EDITOR_VIEWPORT = static_cast<uint32_t>(ViewportType::ProbeEditor);
constexpr uint32_t CHANNEL_SELECTOR_VIEWPORT = static_cast<uint32_t>(ViewportType::ChannelSelector);
constexpr uint32_t VOXEL_VIEWPORT = static_cast<uint32_t>(ViewportType::VoxelViewport);

constexpr uint32_t GetViewportTypeID(ViewportType type) {
	return static_cast<uint32_t>(type);
}


struct RenderView {
	Camera* Camera = nullptr;
	Framebuffer* TargetFBO = nullptr;
};

enum DrawMode {
	DRAW_ELEMENTS = 0,
	DRAW_LINES = 1,
	DRAW_ARRAYS = 2,
	DRAW_POINTS = 3,
	DRAW_ELEMENTS_INSTANCED = 4
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
	// TODO : Implement cleaner solution
	// Temporary solution : If texture == nullptr, use texture_id instead. 
	// Else use TexturePtr->Bind();

	unsigned int texture_id = 0;
	uint32_t Slot = 0; // The texture unit slot to bind to (GL_TEXTURE0 + Slot)

	Texture* TexturePtr = nullptr;	
};

// Renderer:: has a bunch of static functions for API calls
struct RendererAPICall {
	std::function<void()> call;// Here the user puts in the static function or lambda or sometighn
};

struct RenderCommand {
	Shader* ShaderPtr = nullptr; // These are pointers, however we dont own the, and we dont want to care about their lifetime here
	// So we should really not be using pointers, but isntead but either copies or references.
	VertexArray* VAOPtr = nullptr;
	glm::mat4 Transform = glm::mat4(1.0f);

	ViewportType target_viewport = ViewportType::AnatomyViewport;
	DrawMode Mode = DRAW_ELEMENTS;

	uint32_t InstanceCount = 0;  // For instanced rendering

	std::vector<TextureBinding> TextureBindings = {};
	std::vector<UniformData> UniformCommands = {};
	std::vector<RendererAPICall> APICalls = {};
};

struct RendererData
{
	// The queue where systems submit their draw requests
	std::vector<RenderCommand> CommandQueue;
	std::unordered_map<ViewportType, RenderView> ActiveViews;
};

class Renderer {
public:
	static void Init();
	static void Shutdown();

	static void BeginScene();
	static void EndScene();

	static void RegisterView(ViewportType type, const RenderView& rview) {
		sRendererData.ActiveViews[type] = rview;
	}

	static void ExecuteQueue();

	static void Submit(const RenderCommand& command);
	static void Submit(Shader& shader, VertexArray& va, const glm::mat4& transform, ViewportType view, DrawMode mode);

	static void DrawIndexed(const VertexArray* vertexArray, uint32_t indexCount = 0);
	static void DrawIndexedInstanced(const VertexArray* vertexArray, uint32_t instanceCount, uint32_t indexCount = 0);
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
	static RendererData sRendererData;

	static Framebuffer* sCurrentBoundFBO;
	static Camera* sCurrentBoundCamera;
};