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
	// Validate we have everything we need to start projecting ? 

	is_projecting_ = true;

	// Update Probe Projection 
	UpdateProbeProjection();


	// Everyframe we need:

	// We need to know of the Probe

	// We need to know of the Anatomy (Cortex)


	// We need to know of the selected channels

	// On ProjectionTag Changed -> Recalculate Projection Data 
	// Recalculate ActivityLevel Texture read by the projection shader. 
	// Recalculate SensitivityRadius Texture read by the projection shader.		


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

	// We need access to the loaded snirf data ->
	// We need access to the channel intersections calculated in the probe system -> 

	// Bind the activity level texture

}

void ProjectionSystem::UpdateProbeProjection()
{
	// Where does each channel intersect the cortex ? 
	// 1. Load the probe
	const auto& probe = Application::Get().GetSystem<ProbeSystem>()->GetProbe();
	// 2. Load the cortex mesh
	const auto& cortex = NIRS::AnatomyManager::Instance().GetCortex();
	// 3. For each channel, cast a ray from source to detector, find intersection with cortex
	// 4. From this point, find all vertices within the sensitivity radius. 
	// 5. Store these vertices as influenced by this channel.




	// Which vertices are sensitive to which channels ? 

	// How can the fragment shader know which pixels to read from ? 

}


void ProjectionSystem::RenderProjectionCortex()
{
	const auto& cortex = NIRS::AnatomyManager::Instance().GetCortex();

	auto target_viewport = ViewportType::AnatomyViewport;
	auto matrix = cortex->GetTransform()->GetMatrix();
	auto vao = cortex->GetMesh()->GetVAO().get();

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


