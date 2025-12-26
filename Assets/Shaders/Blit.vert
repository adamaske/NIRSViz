#version 330 core

layout (location = 0) in vec2 aPos;      // Vertex Position (NDC space: -1 to 1)
layout (location = 1) in vec2 aTexCoord; // Texture Coordinate (0.0 to 1.0)

out vec2 v_TexCoord;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    v_TexCoord = aTexCoord;
}
