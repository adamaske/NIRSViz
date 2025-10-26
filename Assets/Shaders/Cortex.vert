#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_Transform;
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;

out vec3 v_WorldPosition;
out vec3 v_WorldNormal;

void main()
{
    vec4 modelPos = u_Transform * vec4(aPosition, 1.0);
    v_WorldPosition = modelPos.xyz;

    mat3 NormalMatrix = mat3(transpose(inverse(u_Transform)));
    v_WorldNormal = normalize(NormalMatrix * aNormal);

    gl_Position = u_ProjectionMatrix * u_ViewMatrix * modelPos;
}