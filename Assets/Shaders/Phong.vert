#version 330 core

// Input vertex data from the VAO/VBO
layout (location = 0) in vec4 a_Position; // Vertex position
layout (location = 1) in vec3 a_Normal;   // Vertex normal (must be enabled in your VAO)

// Uniform matrices
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Transform;       // Model Matrix
uniform mat4 u_NormalMatrix;    // Inverse Transpose of ModelView Matrix

// Output to Fragment Shader
out vec3 v_Normal;

void main()
{
    // Transform the vertex position to clip space (standard MVP)
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_Transform * a_Position;

    // Transform the normal vector into View Space (or World Space, but View Space is common)
    // We use the Normal Matrix for this.
    // Note: We only care about the direction, so we use vec3(0.0) for the translation part
    v_Normal = mat3(u_NormalMatrix) * a_Normal;

    // The normal will be automatically interpolated across the face (gouraud shading)
}