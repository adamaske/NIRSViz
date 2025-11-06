#include "pch.h"
#include "App/Layer/FileLayer.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Core/AssetManager.h"
#include "Events/EventBus.h"

#include "NIRS/Anatomy/AnatomyManager.h"

#include "NIRS/Snirf.h"
#include "GUI/SNIRFFileLoaderPanel.h"

FileLayer::FileLayer(const EntityID& settingsID) : Layer(settingsID)
{
}

FileLayer::~FileLayer()
{
}

void FileLayer::OnAttach()
{
	m_SNIRFFileLoaderPanel = CreateRef<SNIRFFileLoaderPanel>();
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
	if (m_SNIRFileLoaderOpen) m_SNIRFFileLoaderPanel->OnImGuiRender(true, m_SNIRFileLoaderOpen);
}

void FileLayer::OnEvent(Event& event)
{
}

void FileLayer::RenderMenuBar()
{
	if (ImGui::BeginMenu("File")) {

		if (ImGui::MenuItem("Open fNIRS file")) {
			m_SNIRFileLoaderOpen = !m_SNIRFileLoaderOpen;
		}

		if (ImGui::MenuItem("Open Head Anatomy")) LoadHeadAnatomy();
		if (ImGui::MenuItem("Open Cortex Anatomy")) LoadCortexAnatomy();

		if (ImGui::MenuItem("Exit")) EventBus::Instance().Publish<ExitApplicationCommand>({});

		ImGui::EndMenu();
	}
}

void FileLayer::PostInit()
{

	std::string snirfFilepath = "C:/dev/NIRSViz/Assets/NIRS/sub01_trial03_TRIM_BP_ZNORM_TDDR.snirf";
	std::string headFilepath = "C:/dev/NIRSViz/Assets/Models/head_model_2.obj";
	std::string cortexFilepath = "C:/dev/NIRSViz/Assets/Models/cortex_model.obj";

	NIRS::AnatomyManager::Instance().LoadCortex(cortexFilepath);
	NIRS::AnatomyManager::Instance().LoadHead(headFilepath);

	AssetManager::Register<SNIRF>("SNIRF", CreateRef<SNIRF>(std::string(snirfFilepath)));
	EventBus::Instance().Publish<OnSNIRFLoaded>({});
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
	
	auto snirf = CreateRef<SNIRF>(std::string(filePath));
	AssetManager::Register<SNIRF>("SNIRF", snirf);

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

	NIRS::AnatomyManager::Instance().LoadHead(std::string(filePath));
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

	NIRS::AnatomyManager::Instance().LoadCortex(std::string(filePath));
}

