#include "pch.h"
#include "App/Layer/FileLayer.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Core/AssetManager.h"
#include "Events/EventBus.h"

#include "NIRS/Snirf.h"

#include "App/Layer/AtlasLayer.h" // TOOD : Move Head and Crotex structs to own header

#include "NIRS/Anatomy/AnatomyManager.h"
#include "NIRS/SNIRFFactory.h"



namespace Utils {
	std::string GetSNIRFFileDialogFilepath() {
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
		if (!GetOpenFileNameA(&ofn)) return "";
		return std::string(filePath);
	};
}

FileLayer::FileLayer(const EntityID& settingsID) : Layer(settingsID)
{
}

FileLayer::~FileLayer()
{
}

void FileLayer::OnAttach()
{
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
	if (m_SNIRFileLoaderOpen) {
		RenderSNIRFFileLoader(true);
	}
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
	auto filePath = Utils::GetSNIRFFileDialogFilepath();

	if (!AssetManager::Get<SNIRF>("SNIRF")) {
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

static SNIRFType SelectedType = SNIRFType::SNIRF_TYPE_UNKNOWN;
static std::string UserSelectedFilepath = "";
static bool IsValid = false; // Example placeholder, replace with member variable
static std::vector<std::string> ValidationErrorMessages;

void FileLayer::RenderSNIRFFileLoader(bool standalone)
{
	if (standalone) {
		// Make the window closeable
		ImGui::Begin("SNIRF File Loader", &m_SNIRFileLoaderOpen);
	}
    // Label on the left
    ImGui::Text("Filepath:");
    ImGui::SameLine();

    // Editable text field (adjust width as needed)
    // NOTE: Resize logic should be outside the render loop for efficiency.
    UserSelectedFilepath.resize(256);
    ImGui::InputTextWithHint("##fileUserInput", "C:/path/to/file.snirf", &UserSelectedFilepath.data()[0], 256, ImGuiInputTextFlags_ReadOnly);

    ImGui::SameLine();
    if (ImGui::Button("Browse")) {
        // Clear old errors
        IsValid = false;
        ValidationErrorMessages.clear();
        // Function to open file dialog and update FilePathUserInput
        LoadSNIRFFile();
    };

    // --- Row 2 : Filetype Selector and Validate Button ---

    const char* previewValue = "Unknown";
    switch (SelectedType) {
    case SNIRFType::SNIRF_TYPE_HOMER3: previewValue = "Homer3"; break;
    case SNIRFType::SNIRF_TYPE_NIRSPY: previewValue = "NIRSpy"; break;
    case SNIRFType::SNIRF_TYPE_SATORI: previewValue = "Satori"; break;
    case SNIRFType::SNIRF_TYPE_AURORA: previewValue = "Aurora"; break;
    case SNIRFType::SNIRF_TYPE_MNE_NIRS: previewValue = "MNE-NIRS"; break;
    case SNIRFType::SNIRF_TYPE_CUSTOM: previewValue = "Custom"; break;
        // default case is covered by initialization to "Unknown"
    }
    ImGui::Text("Filetype:");
    ImGui::SameLine();

    if (ImGui::BeginCombo("##SNIRF_TYPE_COMBO", previewValue)) {
        // Use a small helper function to select and close the combo
        auto SelectSNIRFType = [&](const char* label, SNIRFType type) {
            if (ImGui::Selectable(label)) {
                SelectedType = type;
            }
            // Add a checkmark visual for the currently selected item
            if (SelectedType == type) {
                ImGui::SetItemDefaultFocus();
            }
            };

        SelectSNIRFType("Homer3", SNIRFType::SNIRF_TYPE_HOMER3);
        SelectSNIRFType("NIRSpy", SNIRFType::SNIRF_TYPE_NIRSPY);
        SelectSNIRFType("Satori", SNIRFType::SNIRF_TYPE_SATORI);
        SelectSNIRFType("Aurora", SNIRFType::SNIRF_TYPE_AURORA);
        SelectSNIRFType("MNE-NIRS", SNIRFType::SNIRF_TYPE_MNE_NIRS);
        SelectSNIRFType("Custom", SNIRFType::SNIRF_TYPE_CUSTOM);
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (ImGui::Button("Validate")) {
        // Call your actual validation function.
        // It must set m_IsValid and fill m_ErrorMessages.
        // m_IsValid = ValidateSNIRFFile(FilePathUserInput, SelectedType, m_ErrorMessages);

        // --- Mock Validation for Demonstration ---

        if (UserSelectedFilepath.empty() || SelectedType == SNIRFType::SNIRF_TYPE_NONE) {

            IsValid = false;
            ValidationErrorMessages.push_back("File path is empty.");
            ValidationErrorMessages.push_back("Please select a SNIRF type.");
        }
        else {
            // 
            ValidationErrorMessages.clear();
          

            IsValid = ValidationErrorMessages.empty();
        }
    };

    // --- Row 3 (Conditional): Validation Errors Scrollable Block ---
    if (!IsValid) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Validation Messages:");

        if(ValidationErrorMessages.empty()) {
            ImGui::BeginChild("ValidationLog", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 4), ImGuiChildFlags_Border);
            ImGui::TextWrapped("No messages...");
        }
        else {

            // The second parameter sets the size: 0 width (full width) and 4 lines high.
            if (ImGui::BeginChild("ValidationLog", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 4), ImGuiChildFlags_Border)) {
                for (const auto& msg : ValidationErrorMessages) {
                    // Use ImGui::TextWrapped for better display of long error messages
                    ImGui::TextWrapped("- %s", msg.c_str());
                }
                // Crucially, ImGui::BeginChild creates a scrollable area if the content (ValidationErrorMessages) exceeds this fixed height.
                ImGui::EndChild();
            }
        }
        ImGui::Separator();
    }


    // --- Row 4 (Conditional Color): Open File Button ---

    // Define the colors: Red for failure, Green for success
    ImVec4 buttonColor = IsValid
        ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)  // Green
        : ImVec4(0.5f, 0.2f, 0.2f, 1.0f); // Red

    ImVec4 buttonHoverColor = IsValid
        ? ImVec4(0.2f, 0.9f, 0.2f, 1.0f)
        : ImVec4(0.5f, 0.2f, 0.2f, 1.0f);

    ImVec4 buttonActiveColor = IsValid
        ? ImVec4(0.2f, 0.7f, 0.2f, 1.0f)
        : ImVec4(0.5f, 0.2f, 0.2f, 1.0f);

    // Push the style colors for the button
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);

    // The button takes up the remaining width (-1)
    if (ImGui::Button("Open File", ImVec2(-1, 0))) {
        if (IsValid) {
            // Your actual file opening logic goes here
            // OpenSNIRFFile(FilePathUserInput, SelectedType);

            //auto snirf = SNIRFFactory::CreateSNIRF(SNIRFType::SNIRF_TYPE_SATORI);

            //if (!AssetManager::Get<SNIRF>("SNIRF")) {
            //    AssetManager::Register<SNIRF>("SNIRF", snirf);
            //}
            //
            //snirf->LoadFile(std::string(UserSelectedFilepath));
            //
            //EventBus::Instance().Publish<OnSNIRFLoaded>({});
        }
    }

    // Pop the style colors to prevent them from affecting other elements
    ImGui::PopStyleColor(3);

    if (standalone) {
        ImGui::End();
    }
}
