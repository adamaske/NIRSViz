#version 330 core

// Input vertex attributes from the VAO/VBO
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

// Uniforms provided by the C++ application
uniform mat4 u_Transform;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

void main()
{
    // 1. Transform the position from local (model) space to World space:
    vec4 worldPos = u_Transform * vec4(a_Position, 1.0);

    // 2. Transform the position from World space to Clip space:
    //    (View * Projection * WorldPos)
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * worldPos;
}