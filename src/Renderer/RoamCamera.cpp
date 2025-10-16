#include "pch.h"
#include "Renderer/RoamCamera.h"

#include "Core/Application.h" 
#include "Core/Input.h"
#include "Events/KeyCodes.h"
#include "Events/MouseCodes.h"

#include <glm/gtx/quaternion.hpp>
#include <algorithm> // For std::min/max/clamp

void RoamCamera::OnUpdate(float dt)
{
    if (Input::IsMouseButtonPressed(Mouse::ButtonRight)) {
        if (!m_IsRMBDown)
        {
            // Initial capture: Lock and hide the cursor, record starting mouse position
            m_InitalMousePosition = { Input::GetMouseX(), Input::GetMouseY() };
            m_IsRMBDown = true;

            glfwSetInputMode(Application::Get().GetWindow().GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
        glm::vec2 delta = (mouse - m_InitalMousePosition) * 0.003f;
        m_InitalMousePosition = mouse;

        m_Yaw += delta.x * m_RotationSpeed;
        m_Pitch -= delta.y * m_RotationSpeed; // Subtract because positive Y is usually down on screen

        // Clamp pitch to prevent camera flip (standard 89-degree limit)
        m_Pitch = std::clamp(m_Pitch, -89.9f, 89.9f);
        if(m_Yaw > 360.0f) m_Yaw -= 360.0f;
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
    else {
        if (m_IsRMBDown)
        {
            // Release: Unlock and show the cursor
            m_IsRMBDown = false;

            glfwSetInputMode(Application::Get().GetWindow().GetNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

}

void RoamCamera::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);

    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
        SetViewportSize(e.GetWidth(), e.GetHeight());
        return false;
        });
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
