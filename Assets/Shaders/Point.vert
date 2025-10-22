#version 440 core
layout (location = 0) in vec3 aPosition;

uniform mat4 u_Transform;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

uniform float u_PointSize;
void main()
{
    
    gl_Position = u_ViewMatrix * u_ProjectionMatrix * u_Transform * vec4(aPosition, 1.0);
}