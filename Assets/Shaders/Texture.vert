#version 330 core

layout (location = 0) in vec3 aPos;         // Vertex Position (e.g., -0.5 to 0.5)
layout (location = 1) in vec3 aNormal;         // Vertex Position (e.g., -0.5 to 0.5)
layout (location = 2) in vec2 aTexCoord;    // Vertex Texture Coordinate (e.g., 0.0 to 1.0)

out vec2 v_TexCoord;

uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_Transform; 

void main()
{
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_Transform * vec4(aPos, 1.0);
    
    v_TexCoord = aTexCoord;
}