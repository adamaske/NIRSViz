#pragma once

#include <Core/Base.h>

#include "Renderer/Buffer/VertexBuffer.h"
#include "Renderer/Buffer/IndexBuffer.h"
#include "Renderer/Buffer/BufferLayout.h"
#include "Renderer/Buffer/VertexArray.h"

#include "Renderer/Mesh/MeshGeometry.h"

struct MeshBuffers {
    Ref<VertexArray> vao;
    Ref<VertexBuffer> vbo;
    Ref<IndexBuffer> ibo;

    static MeshBuffers CreateFromGeometry(MeshGeometry& geom);
    void Bind() const { if (vao) vao->Bind(); };
    void Unbind() const { if (vao) vao->Unbind(); };

};
