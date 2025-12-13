#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
class Transform
{
private:
    glm::vec3 m_Position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Rotation{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 model_matrix = glm::mat4(1.0f);
public:

    Transform() = default;


    glm::mat4 GetMatrix() const {

		glm::vec3 rads = glm::radians(m_Rotation);
        glm::quat quat = glm::quat(rads);
        glm::mat4 rotation = glm::toMat4(quat);

        return glm::translate(glm::mat4(1.0f), m_Position)
            * rotation
            * glm::scale(glm::mat4(1.0f), m_Scale);
    }

	glm::vec3& GetPosition() { return m_Position; }
	glm::vec3& GetRotation() { return m_Rotation; }
	glm::vec3& GetScale() { return m_Scale; }

    glm::vec3 GetEulerAnglesRad() const {
        return m_Rotation;
    }

    glm::vec3 GetEulerAnglesDeg() const {
        return glm::degrees(m_Rotation);
    }

    glm::quat GetRotationQuat() const {
        return glm::quat(m_Rotation);
    }

    // Setters
    void SetPosition(const glm::vec3& position) {
        m_Position = position;
    }

    void SetScale(const glm::vec3& scale) {
        m_Scale = scale;
    }

    void SetRotation(const glm::vec3& eulerAnglesRad) {
        m_Rotation = eulerAnglesRad;
    }

    void SetRotationDeg(const glm::vec3& eulerAnglesDeg) {
        m_Rotation = glm::radians(eulerAnglesDeg);
    }

    void SetRotationDeg(float x, float y, float z) {
        m_Rotation = glm::radians(glm::vec3(x, y, z));
    }

    void SetRotationFromQuat(const glm::quat& quat) {
        m_Rotation = glm::eulerAngles(quat);
    }

    // Transform operations
    void Translate(float x, float y, float z) {
        m_Position += glm::vec3(x, y, z);
    }

    void Translate(const glm::vec3& translation) {
        m_Position += translation;
    }

    void Rotate(float angle_degrees, const glm::vec3& axis) {
        // Convert current Euler to quat, apply rotation, convert back
        glm::quat currentQuat = glm::quat(m_Rotation);
        glm::quat rotation = glm::angleAxis(glm::radians(angle_degrees), axis);
        currentQuat = glm::normalize(rotation * currentQuat);
        m_Rotation = glm::eulerAngles(currentQuat);
    }

    void RotateEuler(float x_deg, float y_deg, float z_deg) {
        m_Rotation += glm::radians(glm::vec3(x_deg, y_deg, z_deg));
    }

    void RotateEuler(const glm::vec3& eulerDeg) {
        m_Rotation += glm::radians(eulerDeg);
    }

    void Scale(float x, float y, float z) {
        m_Scale *= glm::vec3(x, y, z);
    }

    void Scale(const glm::vec3& scale) {
        m_Scale *= scale;
    }
};