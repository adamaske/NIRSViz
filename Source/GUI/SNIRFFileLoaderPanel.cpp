#include "pch.h"
#include "GUI/SNIRFFileLoaderPanel.h"

#include <string>
#include <vector>
#include <imgui.h>

#include "Core/AssetManager.h"
#include "Events/EventBus.h"

#include "NIRS/Anatomy/AnatomyManager.h"

#include "NIRS/Snirf.h"
#include "NIRS/SNIRFFactory.h"
#include "NIRS/SNIRFValidator.h"

namespace Utils {
    std::string SNIRFFileDialogFilepath() {
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


SNIRFFileLoaderPanel::SNIRFFileLoaderPanel()
{
	// Set parameters
	IsValid = false;
	SelectedType = SNIRFType::SNIRF_TYPE_NONE;
    UserSelectedFilepath.resize(256);
    ValidationErrorMessages.clear();
}


void SNIRFFileLoaderPanel::OnImGuiRender(bool standalone, bool& open) {

    if (standalone) {
        // Make the window closeable
        auto windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking;
        ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_Once);
        ImGui::Begin("SNIRF File Loader", &open, windowFlags);
    }


    // Label on the left
    ImGui::Text("Filepath:");
    ImGui::SameLine();

    // TODO : Force this column to 300 pixels
    UserSelectedFilepath.resize(256);
    ImGui::InputTextWithHint("##fileUserInput", "C:/path/to/file.snirf", &UserSelectedFilepath.data()[0], 256, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Browse")) {

        UserSelectedFilepath = Utils::SNIRFFileDialogFilepath();
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

    bool CanValidate = std::filesystem::exists(UserSelectedFilepath) && SelectedType != SNIRFType::SNIRF_TYPE_NONE;

    ImVec4 buttonColor = CanValidate
        ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)  // Green
        : ImVec4(0.5f, 0.2f, 0.2f, 1.0f); // Red

    ImVec4 buttonHoverColor = CanValidate
        ? ImVec4(0.2f, 0.9f, 0.2f, 1.0f)
        : ImVec4(0.4f, 0.2f, 0.2f, 1.0f);

    ImVec4 buttonActiveColor = CanValidate
        ? ImVec4(0.2f, 0.7f, 0.2f, 1.0f)
        : ImVec4(0.3f, 0.2f, 0.2f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);

    if (ImGui::Button("Validate")) {

        if (CanValidate) {

            IsValid = SNIRFValidator::Validate(UserSelectedFilepath, SelectedType, ValidationErrorMessages);

        }
        else {
            ValidationErrorMessages.clear();
            IsValid = false;
            ValidationErrorMessages.push_back({ "", "File path is empty." });
            ValidationErrorMessages.push_back({ "", "Please select a SNIRF type." });
        }
    };
    ImGui::PopStyleColor(3);

    // --- Row 3 (Conditional): Validation Errors Scrollable Block ---
    ImGui::Separator();
    ImGui::TextColored(IsValid ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Validation Messages:");

    if (IsValid) {
        ValidationErrorMessages.clear();
        ImGui::BeginChild("ValidationLog", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 4), ImGuiChildFlags_Borders);
        ImGui::TextWrapped("File is valid. Ready for loading.");
        ImGui::EndChild();
    }
    else {

        if (ValidationErrorMessages.empty()) {
            ImGui::BeginChild("ValidationLog", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 4), ImGuiChildFlags_Borders);
            ImGui::TextWrapped("No messages...");
            ImGui::EndChild();
        }
        else {

            // The second parameter sets the size: 0 width (full width) and 4 lines high.
            if (ImGui::BeginChild("ValidationLog", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 4), ImGuiChildFlags_Borders)) {
                for (const auto& msg : ValidationErrorMessages) {
                    // Use ImGui::TextWrapped for better display of long error messages
                    ImGui::TextWrapped("- %s", msg.message.c_str());
                }
                // Crucially, ImGui::BeginChild creates a scrollable area if the content (ValidationErrorMessages) exceeds this fixed height.
                ImGui::EndChild();
            }
        }
        ImGui::Separator();
    }


    // --- Row 4 (Conditional Color): Open File Button ---

    // Define the colors: Red for failure, Green for success
    buttonColor = IsValid
        ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)  // Green
        : ImVec4(0.5f, 0.2f, 0.2f, 1.0f); // Red

    buttonHoverColor = IsValid
        ? ImVec4(0.2f, 0.9f, 0.2f, 1.0f)
        : ImVec4(0.4f, 0.2f, 0.2f, 1.0f);

    buttonActiveColor = IsValid
        ? ImVec4(0.2f, 0.7f, 0.2f, 1.0f)
        : ImVec4(0.3f, 0.2f, 0.2f, 1.0f);

    // Push the style colors for the button
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonActiveColor);

    // The button takes up the remaining width (-1)
    if (ImGui::Button("Open File", ImVec2(-1, 0))) {


        auto snirf = SNIRFFactory::CreateSNIRF(SelectedType, UserSelectedFilepath);
        AssetManager::Register<SNIRF>("SNIRF", snirf);
        EventBus::Instance().Publish<OnSNIRFLoaded>({});

        if (!IsValid || !CanValidate) { // We opened an invalid file, dont close window and show warning

            ValidationErrorMessages.clear();
            ValidationErrorMessages.push_back({ "", "File is invalid. Attempting to open..." });
        }
        else {
            open = false; // We opened a valid file, close window afterwards
        }
    }

    ImGui::PopStyleColor(3);

    if (standalone) {
        ImGui::End();
    }
}