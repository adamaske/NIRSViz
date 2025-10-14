#include "pch.h"
#include "ProbeLayer.h"

#include "Core/Application.h"

#include <imgui.h>

namespace Utils {
    std::string OpenSNIRFFileDialog() {
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

        if (GetOpenFileNameA(&ofn)) {
            return std::string(filePath);
        }
        return {};
    }

}

ProbeLayer::ProbeLayer() : Layer("ProbeLayer")
{
}

ProbeLayer::~ProbeLayer()
{
}

void ProbeLayer::OnAttach()
{
    
}

void ProbeLayer::OnDetach()
{
}

void ProbeLayer::OnUpdate(float dt)
{
    if (m_DrawProbe && m_ProbeLoaded) {

    }

  
}

void ProbeLayer::OnRender()
{
}

void ProbeLayer::OnImGuiRender()
{
	ImGui::Begin("Probe Settings");
    if (m_ProbeLoaded) 
        RenderProbeInformation();
    else
		LoadProbeButton();
    ImGui::End();
}

void ProbeLayer::OnEvent(Event& event)
{
}

void ProbeLayer::RenderProbeInformation()
{
    ImGui::Text("Loaded Probe File:");
    ImGui::SameLine();
    if (ImGui::Button("Reload")) {
        std::string newFile = Utils::OpenSNIRFFileDialog();
        if (!newFile.empty()) {
            LoadProbeFile(newFile);
        }
    }
    ImGui::TextWrapped("%s", m_CurrentFilepath.c_str());
    ImGui::Checkbox("Draw Probe", &m_DrawProbe);
}

void ProbeLayer::LoadProbeButton()
{
    // This is only called without a loaded probe
    ImGui::Text("No .SNIRF file loaded.");
    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
        std::string newFile = Utils::OpenSNIRFFileDialog();
        if (!newFile.empty()) {
            LoadProbeFile(newFile);
        }
    }
}

void ProbeLayer::LoadProbeFile(const std::string& filepath)
{   
	NVIZ_ASSERT(std::filesystem::exists(filepath), "File does not exist");
	m_CurrentFilepath = filepath;


	m_ProbeLoaded = true;
}
