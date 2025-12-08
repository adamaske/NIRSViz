#include "pch.h"
#include "GUI/GUI.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/geometric.hpp>
#include "glm/gtx/string_cast.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

void GUI::RenderVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth)
{
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();
	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize))
		values.x = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize))
		values.y = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize))
		values.z = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);

	ImGui::PopID();
}

void GUI::RenderTransformSettings(Transform* transform)
{
	auto& position = transform->GetPosition();
	auto& rotation = transform->GetRotation();
	auto& scale = transform->GetScale();

	RenderVec3Control("Position", position, 0.0f);
	RenderVec3Control("Rotation", rotation, 0.0f);
	RenderVec3Control("Scale", scale, 1.0f);
}

void GUI::RenderAnatomySettingsImpl(Anatomy* anatomy, const std::string& name, const std::string& label, bool standalone)
{
	//NVIZ_ASSERT(std::is_base_of<Anatomy, T>::value, "RenderAnatomySettings: T must be a subclass of Anatomy");

	ImGui::PushID(std::string("anatomy_settings_" + name).c_str());
	if (standalone)
		ImGui::Begin(label.c_str());

	// Visiblity Toggle
	bool visible = anatomy->IsVisible();
	if (ImGui::Checkbox(std::string("Show " + name).c_str(), &visible))
	{
		anatomy->SetVisible(visible);
	}
	bool opened = true; // Default to opened, but if not standalone, use CollapsingHeader
	if (!standalone) opened = ImGui::CollapsingHeader(label.c_str());

	if (opened)
	{
		// --- INFO ---
		ImGui::TextDisabled("Info:");
		ImGui::Separator();
		auto mesh = anatomy->GetMesh();
		ImGui::TextDisabled("Vertices: %d", mesh->GetVertexCount());
		ImGui::TextDisabled("Indices: %d", mesh->GetIndexCount());
		ImGui::Spacing();
		ImGui::TextDisabled("Mesh File: %s", anatomy->GetMeshFilepath().c_str());
		ImGui::Separator();
		ImGui::Spacing();

		// --- VISUALIZATION ---
		// Opacity Slider
		float opacity = anatomy->GetOpacity();
		if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f))
		{
			anatomy->SetOpacity(opacity);
		}

		ImGui::Spacing();
		// Position, Rotation, Scale Controls
		RenderTransformSettings(anatomy->GetTransform().get());
	}

	if (standalone)
		ImGui::End();

	ImGui::PopID();
}

void GUI::RenderLineRendererSettings(LineRenderer* renderer, bool& show, const std::string& label, bool standalone, float columnWidth)
{
	ImGui::PushID(std::string(label + "_settings").c_str());

	ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs;

	ImGui::TextDisabled(std::string(label + " Settings").c_str());
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnWidth(0, columnWidth * 0.75); // Label column width
	std::string showText = std::string("Show " + label).c_str();
	ImGui::Text(std::string("Show " + label).c_str());
	ImGui::SameLine();
	ImGui::Checkbox(std::string("##" + showText).c_str(), &show);
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1); // Fill remaining space
	ImGui::Text("Color");
	ImGui::SameLine();
	ImGui::ColorEdit4("##Color", &renderer->m_LineColor[0], colorFlags);
	ImGui::NextColumn();
	//ImGui::PushItemWidth(-1); // Fill remaining space
	ImGui::Text("Width");
	ImGui::SameLine();
	ImGui::SliderFloat("##Width", &renderer->m_LineWidth, 1.0f, 10.0f);

	ImGui::PopItemWidth();
	ImGui::PopID();
}

void GUI::RenderPointRendererSettings(LineRenderer* renderer, bool& show, const std::string& label, bool standalone, float columnWidth)
{
}

void GUI::RenderSNIRFInfo(SNIRF* snirf)
{
	auto& probe = snirf->GetProbe();

	ImGui::PushID("snirf_info");
	ImGui::TextDisabled("SNIRF File Info:");
	ImGui::Separator();
	ImGui::TextDisabled("Filepath: %s", snirf->GetFilepath().c_str());
	ImGui::TextDisabled("Sources: %d", probe.sources.size());
	ImGui::TextDisabled("Detectors: %d", probe.detectors.size());
	ImGui::TextDisabled("Channels: %d", probe.channels.size()); // Updated to use 'probe.channels.size()'
	ImGui::TextDisabled("Wavelengths: %d", snirf->GetWavelengths().size());
	ImGui::TextDisabled("Sampling Rate: %.2f Hz", snirf->GetSamplingRate());
	ImGui::TextDisabled("Duration: %.2f seconds", snirf->GetDurationSeconds());
	ImGui::Separator();
	ImGui::PopID();
}
