#include "pch.h"
#include "Renderer/Mesh/MeshBuffers.h"

MeshBuffers MeshBuffers::CreateFromGeometry(MeshGeometry &geom) {
    MeshBuffers buffer;

    buffer.vao = CreateRef<VertexArray>();
    buffer.vao->Bind();

    buffer.vbo = CreateRef<VertexBuffer> (&geom.vertices[0], geom.vertices.size() * sizeof(Vertex));
    buffer.ibo = CreateRef<IndexBuffer>(&geom.indices[0], geom.indices.size());

    BufferElement pos = {ShaderDataType::Float3, "aPos", false};
    BufferElement norms = {ShaderDataType::Float3, "aNormal", false};
    BufferElement cords = {ShaderDataType::Float2, "aTexCoord", false};
    BufferLayout layout = BufferLayout{pos, norms, cords};
    buffer.vbo->SetLayout(layout);

    buffer.vao->AddVertexBuffer(buffer.vbo);
    buffer.vao->SetIndexBuffer(buffer.ibo);

    return buffer;
}