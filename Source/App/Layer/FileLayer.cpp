#include "pch.h"
#include "App/Layer/FileLayer.h"

#include <imgui.h>

#include "Core/AssetManager.h"
#include "Events/EventBus.h"

#include "NIRS/Snirf.h"

#include "App/Layer/AtlasLayer.h" // TOOD : Move Head and Crotex structs to own header

FileLayer::FileLayer(const EntityID& settingsID) : Layer(settingsID)
{
}

FileLayer::~FileLayer()
{
}

void FileLayer::OnAttach()
{
	std::string snirfFilepath = "C:/dev/NIRSViz/Assets/NIRS/sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf";
	std::string headFilepath = "C:/dev/NIRSViz/Assets/Models/head_model_2.obj";
	std::string cortexFilepath = "C:/dev/NIRSViz/Assets/Models/cortex_model.obj";

	// HEAD ANATOMY
	Head head;
	head.Mesh = CreateRef<Mesh>(headFilepath);
	head.Transform = CreateRef<Transform>();
	head.Graph = CreateRef<Graph>(CreateGraphFromTriangleMesh(head.Mesh.get(), glm::mat4(1.0f)));

	head.MeshFilepath = headFilepath;

	AssetManager::Register<Head>("Head", CreateRef<Head>(head));
	EventBus::Instance().Publish<HeadAnatomyLoadedEvent>({ });

	// CORTEX ANATOMY
	Cortex cortex;
	cortex.Mesh = CreateRef<Mesh>(cortexFilepath);
	cortex.Transform = CreateRef<Transform>();
	cortex.Graph = CreateRef<Graph>(CreateGraphFromTriangleMesh(cortex.Mesh.get(), glm::mat4(1.0f)));
	cortex.MeshFilepath = cortexFilepath;

	AssetManager::Register<Cortex>("Cortex", CreateRef<Cortex>(cortex));
	EventBus::Instance().Publish<CortexAnatomyLoadedEvent>({});


	// SNIRF
	Ref<SNIRF> snirf = CreateRef<SNIRF>(snirfFilepath);
	AssetManager::Register<SNIRF>("SNIRF", snirf);
	EventBus::Instance().Publish<OnSNIRFLoaded>({});
}

void FileLayer::OnDetach()
{
}

void FileLayer::OnUpdate(float dt)
{
}

void FileLayer::OnRender()
{
}

void FileLayer::OnImGuiRender()
{
	
}

void FileLayer::OnEvent(Event& event)
{
}

void FileLayer::RenderMenuBar()
{
	if (ImGui::BeginMenu("File")) {



		if(ImGui::MenuItem("Open fNIRS file")) {
			// On Load SNIRF File
			LoadSNIRFFile();
		}
		if (ImGui::MenuItem("Open Head Anatomy")) {
			// On Load SNIRF File
			LoadHeadAnatomy();
		}
		if (ImGui::MenuItem("Open Cortex Anatomy")) {
			// On Load SNIRF File
			LoadCortexAnatomy();
		}

		if (ImGui::MenuItem("Exit")) {

			// On Exit
			EventBus::Instance().Publish<ExitApplicationCommand>({});
		}

		ImGui::EndMenu();
	}
}

void FileLayer::PostInit()
{
	

}

void FileLayer::LoadSNIRFFile()
{
	char filePath[MAX_PATH] = "";
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = sizeof(filePath);
	ofn.lpstrFilter = "SNIRF Files (*.snirf)\0*.snirf\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileNameA(&ofn)) return;

	if (!AssetManager::Get<SNIRF>("SNIRF")) { //
		AssetManager::Register<SNIRF>("SNIRF", CreateRef<SNIRF>());
	}

	auto snirf = AssetManager::Get<SNIRF>("SNIRF");
	snirf->LoadFile(std::string(filePath));

	EventBus::Instance().Publish<OnSNIRFLoaded>({});
}

void FileLayer::LoadHeadAnatomy()
{

	char filePath[MAX_PATH] = "";

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = sizeof(filePath);
	ofn.lpstrFilter = "OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileNameA(&ofn)) return;

	Head head;

	head.Mesh = CreateRef<Mesh>(std::string(filePath));
	head.Transform = CreateRef<Transform>();
	head.Graph = CreateRef<Graph>(CreateGraphFromTriangleMesh(head.Mesh.get(), glm::mat4(1.0f)));

	head.MeshFilepath = std::string(filePath);

	AssetManager::Register<Head>("Head", CreateRef<Head>(head));
	EventBus::Instance().Publish<HeadAnatomyLoadedEvent>({ });
}

void FileLayer::LoadCortexAnatomy()
{

	char filePath[MAX_PATH] = "";

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = sizeof(filePath);
	ofn.lpstrFilter = "OBJ Files (*.obj)\0*.obj\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileNameA(&ofn)) return;

	Cortex cortex;
	cortex.Mesh = CreateRef<Mesh>(std::string(filePath));
	cortex.Transform = CreateRef<Transform>();
	cortex.Graph = CreateRef<Graph>(CreateGraphFromTriangleMesh(cortex.Mesh.get(), glm::mat4(1.0f)));
	cortex.MeshFilepath = std::string(filePath);

	AssetManager::Register<Cortex>("Cortex", CreateRef<Cortex>(cortex));
	EventBus::Instance().Publish<CortexAnatomyLoadedEvent>({});
}
