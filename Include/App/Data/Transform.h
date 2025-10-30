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
    glm::quat m_Rotation{ 0.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 model_matrix = glm::mat4(1.0f);
public:

    Transform() = default;

    glm::mat4 GetMatrix() const {

        glm::mat4 rotation = glm::toMat4(glm::quat(m_Rotation));

        return glm::translate(glm::mat4(1.0f), m_Position)
            * rotation
            * glm::scale(glm::mat4(1.0f), m_Scale);
    }

	glm::vec3 GetPosition() const { return m_Position; }
	glm::quat GetRotation() const { return m_Rotation; }
	glm::vec3 GetScale() const { return m_Scale; }

    void SetPosition(glm::vec3 position) {
        m_Position = position;
	};
    void SetScale(glm::vec3 scale){
        m_Scale = scale;
	};
    void SetRotation(glm::quat rotation) {
        m_Rotation = rotation;
    };

    void Translate(float x, float y, float z) {
		m_Position += glm::vec3(x, y, z);
    };
    void Translate(glm::vec3 translation) {
        m_Position += translation;
    };

    void Rotate(float angle_degrees, const glm::vec3& axis) {
        m_Rotation = glm::normalize(glm::rotate(m_Rotation,
            glm::radians(angle_degrees),
            axis));
    };

    void Scale(float x, float y, float z) {
		m_Scale *= glm::vec3(x, y, z);
    };
    void Scale(glm::vec3 scale) {
        m_Scale *= scale;
    };

};