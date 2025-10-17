#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec3 LightPos;

uniform vec3 u_LightPos = vec3(0, 100, 0); // we now define the uniform in the vertex shader and pass the 'view space' lightpos to the fragment shader. lightPos is currently in world space.

uniform mat4 u_Transform;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

void main()
{
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_Transform * vec4(aPos, 1.0);
    FragPos = vec3(u_ViewMatrix * u_Transform * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_ViewMatrix * u_Transform))) * aNormal;
    LightPos = vec3(u_ViewMatrix * vec4(lightPos, 1.0)); // Transform world-space light position to view-space light position
}