#pragma once
#include "Core/Base.h"

#include "Renderer/Renderable/Mesh.h"
#include "App/Data/Transform.h"
#include "App/Data/MeshGraph.h"

class Anatomy {
public:
    Anatomy(std::string& meshPath);

    const Ref<Mesh>& GetMesh() const { return m_Mesh; }
    const Ref<Transform>& GetTransform() const { return m_Transform; }
    const Ref<Graph>& GetGraph() const { return m_Graph; }


    std::vector<glm::vec3> GetWorldSpaceVertexPositions() const;
    unsigned int FindClosestVertex(const glm::vec3& position) const;

    bool& IsVisible() { return m_Visible; }

    void SetVisible(bool visible) { m_Visible = visible; }
    float& GetOpacity() { return m_Opacity; }
    void SetOpacity(float opacity) { m_Opacity = opacity; }
private:
    Ref<Mesh> m_Mesh;
    Ref<Transform> m_Transform;
    Ref<Graph> m_Graph;
    std::string m_MeshFilepath;

    bool m_Visible = true;
    float m_Opacity = 1.0f;
};