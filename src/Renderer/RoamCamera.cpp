#include "pch.h"
#include "Renderer/RoamCamera.h"

#include "Core/Application.h" 
#include "Core/Input.h"
#include "Events/KeyCodes.h"
#include "Events/MouseCodes.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/quaternion.hpp>
#include <algorithm> // For std::min/max/clamp

void RoamCamera::OnUpdate(float dt)
{
}

void RoamCamera::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);

    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
        SetViewportSize(e.GetWidth(), e.GetHeight());
        return false;
    });
}

void RoamCamera::OnImGuiRender(bool standalone)
{
    if (standalone) ImGui::Begin("Roam Camera Settings");

    ImGui::SliderFloat("Movement Speed", &m_MovementSpeed, 0.0f, 500.0f);
    ImGui::SliderFloat("Rotation Speed", &m_RotationSpeed, 0.0f, 500.0f);

    const glm::vec3& pos = GetPosition();
    ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    const glm::vec3& front = GetFront();
    ImGui::Text("Front:    %.2f, %.2f, %.2f", front.x, front.y, front.z);
    const glm::vec3& right = GetRight();
    ImGui::Text("Right:    %.2f, %.2f, %.2f", right.x, right.y, right.z);
    const glm::vec3& up = GetUp();
    ImGui::Text("Up:       %.2f, %.2f, %.2f", up.x, up.y, up.z);
    ImGui::Text("Pitch:    %.2f", GetPitch());
    ImGui::Text("Yaw:      %.2f", GetYaw());

	if (standalone) ImGui::End();
}

void RoamCamera::StartControl(glm::vec2 initalPos)
{
	m_InitalMousePosition = initalPos;
}

void RoamCamera::OnControlled(float dt)
{
    const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
    glm::vec2 delta = (mouse - m_InitalMousePosition) * 0.003f;
   // m_InitalMousePosition = mouse;

    m_Yaw += delta.x * m_RotationSpeed;
    m_Pitch -= delta.y * m_RotationSpeed; // Subtract because positive Y is usually down on screen

    // Clamp pitch to prevent camera flip (standard 89-degree limit)
    m_Pitch = std::clamp(m_Pitch, -89.9f, 89.9f);
    if (m_Yaw > 360.0f) m_Yaw -= 360.0f;
    else if (m_Yaw < 0.0f) m_Yaw += 360.0f;

    UpdateCameraVectors();

    float velocity = m_MovementSpeed * dt;

    // Optional: Increase speed with LeftShift (Unreal default)
    if (Input::IsKeyPressed(Key::LeftShift))
        velocity *= 3.0f; // Faster movement

    // W & S: Forward/Backward (along local m_Front vector)
    if (Input::IsKeyPressed(Key::W))
        m_Position += front * velocity;
    if (Input::IsKeyPressed(Key::S))
        m_Position -= front * velocity;

    // A & D: Strafe Left/Right (along local m_Right vector)
    if (Input::IsKeyPressed(Key::A))
        m_Position -= right * velocity;
    if (Input::IsKeyPressed(Key::D))
        m_Position += right * velocity;

    // E & Q: World Up/Down (along world Y axis)
    if (Input::IsKeyPressed(Key::E))
        m_Position += WORLD_UP * velocity;
    if (Input::IsKeyPressed(Key::Q))
        m_Position -= WORLD_UP * velocity;

    UpdateViewMatrix();
}

void RoamCamera::UpdateCameraVectors() {
    glm::vec3 front_temp;

    front_temp.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front_temp.y = sin(glm::radians(m_Pitch));
    front_temp.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    front = glm::normalize(front_temp);
    right = glm::normalize(glm::cross(front, WORLD_UP));
    up = glm::normalize(glm::cross(right, front));
}

void RoamCamera::UpdateViewMatrix()
{
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + front, up);
}

void RoamCamera::UpdateProjectionMatrix()
{
    m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
}
