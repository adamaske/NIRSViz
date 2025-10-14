#version 440 core

layout (location = 0) in vec3 aPos;      // vertex position
layout (location = 1) in vec3 aNormal;   // vertex normal
layout (location = 2) in vec2 aTexCoord; // optional UVs

out vec3 FragPos;   // position in world space
out vec3 Normal;    // normal in world space
out vec2 TexCoord;  // pass UVs to fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transform vertex position to world space
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Correctly transform normals (use transpose-inverse of model for scaling)
    Normal = mat3(transpose(inverse(model))) * aNormal;

    TexCoord = aTexCoord;

    // Final clip space position
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
