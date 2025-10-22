#include "pch.h"
#include "Renderer/OrbitCamera.h"

#include "Core/Application.h" 
#include "Core/Input.h"
#include "Events/KeyCodes.h"
#include "Events/MouseCodes.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

OrbitCamera::OrbitCamera() : Camera()
{
	SetOrbitPosition("Default");
}
OrbitCamera::~OrbitCamera()
{
}
void OrbitCamera::OnUpdate(float dt)
{
}

void OrbitCamera::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
        SetViewportSize(e.GetWidth(), e.GetHeight());
        return false;
        });
}

void OrbitCamera::OnImGuiRender(bool standalone)
{
    if (standalone) ImGui::Begin("Roam Camera Settings");

    if (ImGui::SliderFloat("Orbit Distance", &m_Radius, 0.1f, 150.0f)) {
        SetOrbitPosition(m_Theta, m_Phi, m_Radius);
    }
    if (ImGui::SliderFloat("Theta", &m_Theta, -360.0f, 360.0f)) {
        SetOrbitPosition(m_Theta, m_Phi, m_Radius);
    }
    if (ImGui::SliderFloat("Phi", &m_Phi, -90.0f, 90.0f)) {
        SetOrbitPosition(m_Theta, m_Phi, m_Radius);
    }
    for (int i = 0; i < orbit_positions.size(); i++) {
        auto& pos = orbit_positions[i];
        if (ImGui::Button(std::get<0>(pos).c_str())) {
            SetOrbitPosition(std::get<0>(pos));
        }
        if (ImGui::IsItemHovered()) {
            std::tuple<float, float> t_p = std::get<1>(pos);
            float t = std::get<0>(t_p);
            auto p = std::get<1>(t_p);
            ImGui::SetTooltip("Theta: %.1f, Phi: %.1f", t, p);
        }

        if (i % 3 != 0) ImGui::SameLine();
    }
    if (standalone) ImGui::End();
}

void OrbitCamera::UpdateViewMatrix()
{
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + front, up);
}

void OrbitCamera::UpdateProjectionMatrix()
{
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
}

void OrbitCamera::InsertTarget(const std::string& name, Transform* target)
{
    orbit_target_map[name] = target;
}

void OrbitCamera::SetCurrentTarget(const std::string& name)
{
    if (orbit_target_map.find(name) != orbit_target_map.end()) {
        orbit_target = orbit_target_map[name];
        orbit_target_name = name;
    };
}

void OrbitCamera::SetOrbitPosition(float _theta, float _phi, float _distance)
{
    glm::vec3 target_pos = glm::vec3(0.0f);
    if (orbit_target) {
        target_pos = orbit_target->GetPosition();
    }

    _phi = glm::clamp(_phi, -89.99f, 89.99f);
    float theta = glm::radians(_theta);
    float phi = glm::radians(_phi);

    float x = _distance * cos(phi) * cos(theta);
    float y = _distance * sin(phi);
    float z = _distance * cos(phi) * sin(theta);

    m_Position = target_pos + glm::vec3(x, y, z);

    front = glm::normalize(target_pos - m_Position);
    right = glm::normalize(glm::cross(front, WORLD_UP));
    up = glm::normalize(glm::cross(right, front));

    m_Theta = _theta;
    m_Phi = _phi;
	m_Radius = _distance;

    UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void OrbitCamera::SetOrbitPosition(const std::string& _name)
{
    for (auto& [name, pos] : orbit_positions) {
        if (name == _name) {
			SetOrbitPosition(std::get<0>(pos), std::get<1>(pos), m_Radius);
        }
    }
}

void OrbitCamera::SetTheta(float _theta)
{
	m_Theta = _theta;
	SetOrbitPosition(m_Theta, m_Phi, m_Radius);
}

void OrbitCamera::SetPhi(float _phi)
{
    m_Phi = _phi;
	SetOrbitPosition(m_Theta, m_Phi, m_Radius);
}

void OrbitCamera::SetRadius(float _radius)
{
    m_Radius = _radius;
	SetOrbitPosition(m_Theta, m_Phi, m_Radius);
}
