#pragma once
#include <ranges>

#include "Core/Base.h"

#include "../../Renderer/Mesh/Mesh.h"
#include "App/Data/Transform.h"
#include "App/Data/MeshGraph.h"

class Anatomy {
public: // TODO : Currently we have to supply a valid obj_filepath to create anatomy,
    // rather the mesh should be opt-in.
    Anatomy(std::filesystem::path obj_filepath);
    ~Anatomy() = default;

    Ref<Graph> GetGraph() { return m_Graph; }


    std::vector<glm::vec3> GetWorldSpaceVertexPositions() const;
    unsigned int FindClosestVertex(const glm::vec3& position) const;

    bool& IsVisible() { return m_Visible; }

    void SetVisible(bool visible) { m_Visible = visible; }
    float& GetOpacity() { return m_Opacity; }
    void SetOpacity(float opacity) { m_Opacity = opacity; }

	std::string GetMeshFilepath() const { return mesh_.fd.filepath.string(); }

    // TODO : This should overtake the Ref's
    const Mesh& GetMesh() { return mesh_; }
    Mesh& GetMeshMutable() { return mesh_; }

    const Transform& GetTransform() { return transform_; };
    Transform& GetTransformMutable() { return transform_; };
    // To account for the world-local space dynamic as a concsequence of the transform compoent
    // We need to wrap the spatial index intersection functions
    bool IntersectAnatomy(const Ray& ray, RayHit& out_hit);

private:
    Mesh mesh_;
    Transform transform_;

    // TODO : Implement MeshTopology instead..
    Ref<Graph> m_Graph;

    bool m_Visible = true;
    float m_Opacity = 1.0f;
};