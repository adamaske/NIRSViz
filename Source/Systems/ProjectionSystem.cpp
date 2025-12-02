#include "pch.h"
#include "Systems/ProjectionSystem.h"

#include "Core/Application.h"

#include "Events/EventBus.h"	
#include "NIRS/Anatomy/AnatomyManager.h"

#include "Renderer/Renderer.h"
#include "Renderer/Renderable/Texture.h"
#include "Renderer/ViewportManager.h"



ProjectionSystem::ProjectionSystem()
{
}

ProjectionSystem::~ProjectionSystem()
{
}

void ProjectionSystem::OnAttach()
{

	projection_shader_ = Shader(
		"C:/dev/NIRSViz/Assets/Shaders/VertexProjection.vert", 
		"C:/dev/NIRSViz/Assets/Shaders/VertexProjection.frag");

	SetupSubscriptions();

}

void ProjectionSystem::OnDetach()
{
}

void ProjectionSystem::OnUpdate(DeltaTime dt)
{
	if (!is_projecting_)
		return;

	// 1. Render the cortex -> Render it to the main viewport framebuffer. 
	RenderProjectionCortex();
}

void ProjectionSystem::OnGUIRender() {


}

void ProjectionSystem::OnEvent(Event& event) {
}

void ProjectionSystem::RenderMenuBar() {
}

void ProjectionSystem::SetupSubscriptions()
{
	auto& bus = EventBus::Instance();

	// TODO : Defer this command to a projection queue to avoid threading issues
	bus.Subscribe<OnUserStartProjectionCommand>([&](const OnUserStartProjectionCommand& e) {
		StartProjection();
		});

	bus.Subscribe<OnProjectionTimeChanged>([&](const OnProjectionTimeChanged& e) {
		// Update projection time in settings
		NVIZ_ERROR("ProjectionSystem received OnProjectionTimeChanged: index {0}, actual {1}", e.time_index, e.time_seconds);
		NVIZ_ERROR("AVOID THIS METHOD -> The projectionsystem should be called directly...");
	});
}

void ProjectionSystem::StartProjection()
{
	is_projecting_ = true;

	// We need to spin up our own vertex array with the strength 
	SetupCortexRendering();


	UpdateInfluenceMap();
}

void ProjectionSystem::StopProjection()
{
	is_projecting_ = false;

	// Shutdown logic
}

void ProjectionSystem::SetProjectionWavelength(const NIRS::WavelengthType& wavelength)
{
	wavelength_ = wavelength;
}

void ProjectionSystem::UpdateProjectionTimeIndex(size_t index, double actual)
{
	settings_.time_index = index;

	UpdateActivatedVertices();
	// We need access to the loaded snirf data ->
	// We need access to the channel intersections calculated in the probe system -> 

	// Bind the activity level texture

}



void ProjectionSystem::RenderProjectionCortex()
{
	const auto& cortex = NIRS::AnatomyManager::Instance().GetCortex();

	auto target_viewport = ViewportType::AnatomyViewport;
	auto matrix = cortex->GetTransform()->GetMatrix();
	auto vao = cortex_vao_.get();

	UniformData lightPos; // TODO : Move to shared uniform buffer ? 
	lightPos.Type = UniformDataType::FLOAT3;
	lightPos.Name = "u_LightPos";
	lightPos.Data.f3 = ViewportManager::GetViewport(target_viewport).Camera->GetPosition();

	UniformData objectColor;
	objectColor.Type = UniformDataType::FLOAT4;
	objectColor.Name = "u_ObjectColor";
	objectColor.Data.f4 = { 0.4f, 0.4f, 0.4f, 1.0f };

	UniformData strengthMin;
	strengthMin.Type = UniformDataType::FLOAT1;
	strengthMin.Name = "u_StrengthMin";
	strengthMin.Data.f1 = settings_.StrengthMin;

	UniformData strengthMax;
	strengthMax.Type = UniformDataType::FLOAT1;
	strengthMax.Name = "u_StrengthMax";
	strengthMax.Data.f1 = settings_.StrengthMax;

	UniformData ambientStrength;
	ambientStrength.Type = UniformDataType::FLOAT1;
	ambientStrength.Name = "u_AmbientStrength";
	ambientStrength.Data.f1 = 0.4f;


	RenderCommand cmd;
	cmd.ShaderPtr = &projection_shader_;
	cmd.VAOPtr = vao; // TODO : Determine if we need 
	cmd.Transform = matrix;

	cmd.target_viewport = target_viewport;	
	cmd.Mode = DRAW_ELEMENTS;
	
	cmd.UniformCommands = {lightPos, objectColor, strengthMin, strengthMax, ambientStrength };
	cmd.APICalls = {};

	Renderer::Submit(cmd);
}

void ProjectionSystem::UpdateInfluenceMap()
{
	// Where does each channel intersect the cortex ? 
	// 1. Load the probe
	// 2. Load the cortex mesh
	// 3. For each channel, cast a ray from source to detector, find intersection with cortex
	// 4. From this point, find all vertices within the sensitivity radius. 
	// 5. Store these vertices as influenced by this channel.

	// Specifically we want the visual probe / the user modified probe -> 

	const auto& probe_system = Application::Get().GetSystem<ProbeSystem>();
	const auto& intersections = probe_system->GetChannelProjectionResult();

	const auto& cortex = NIRS::AnatomyManager::Instance().GetCortex();
	const auto& vertices = cortex->GetMesh()->GetVertices();
	const auto& transform = cortex->GetTransform()->GetMatrix();

	influenced_vertices_.clear();

	for (auto& [channel_id, intersection_point] : intersections) {

		// We want to both know wheter the vertex is wihtin the influence radius/
		// And we want the actual distance to calculate infleunce falloff.
		std::vector<int> influenced_vertices;
		std::vector<double> distances;


		for (int i = 0; i < vertices.size(); i++) {

			// We need the world position of the vertex
			glm::vec3 world_pos = transform * glm::vec4(vertices[i].position, 1.0f);

			float distance = glm::distance(intersection_point, world_pos);
			if (distance <= settings_.Radius) {

				InfluencedVertex iv = { i, distance };
				influenced_vertices_[channel_id].push_back(iv);
			}
		}
	}
}

void ProjectionSystem::UpdateActivatedVertices()
{

}

void ProjectionSystem::SetupCortexRendering()
{
	auto cortex = NIRS::AnatomyManager::Instance().GetCortex();

	auto vertices = cortex->GetMesh()->GetVertices();
	auto indices = cortex->GetMesh()->GetIndices();

	projection_vertices_.resize(vertices.size());

	for (int i = 0; i < vertices.size(); i++)
	{
		projection_vertices_[i].position = vertices[i].position;
		projection_vertices_[i].normal = vertices[i].normal;
		projection_vertices_[i].tex = vertices[i].tex_coords;
		projection_vertices_[i].activity_level = 0.0f; // Initialize activity level
	}

	cortex_vao_ = CreateRef<VertexArray>();
	cortex_vao_->Bind();

	auto vbo = CreateRef<VertexBuffer>(&projection_vertices_[0], projection_vertices_.size() * sizeof(ProjectionVertex));
	auto ibo = CreateRef<IndexBuffer>(&indices[0], (unsigned int)(indices.size()));

	BufferElement pos = { ShaderDataType::Float3, "aPos", false };
	BufferElement norms = { ShaderDataType::Float3, "aNormal", false };
	BufferElement cords = { ShaderDataType::Float2, "aTexCoord", false };
	BufferElement activity = { ShaderDataType::Float, "aActivityLevel", false };
	BufferLayout layout = BufferLayout{ pos, norms, cords, activity };

	vbo->SetLayout(layout);

	cortex_vao_->AddVertexBuffer(vbo);
	cortex_vao_->SetIndexBuffer(ibo);

	cortex_vao_->Unbind();
}
