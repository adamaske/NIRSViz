#version 330 core

// Per-vertex attributes (unit cube)
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;

// Per-instance attributes (voxel data)
layout (location = 2) in vec3 a_InstancePosition;
layout (location = 3) in float a_InstanceIntensity;

// Outputs to fragment shader
out vec3 v_FragPos;
out vec3 v_Normal;
out vec3 v_WorldPos;
out float v_Intensity;

// Uniforms
uniform mat4 u_ViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform float u_VoxelSize = 1.0;

void main()
{
    // Scale the unit cube by voxel size and translate to instance position
    vec3 scaledPos = a_Position * u_VoxelSize;
    vec3 worldPos = scaledPos + a_InstancePosition;
    v_WorldPos = worldPos;

    // Calculate view space position
    vec4 viewPos = u_ViewMatrix * vec4(worldPos, 1.0);
    v_FragPos = viewPos.xyz;

    // Calculate final position
    gl_Position = u_ProjectionMatrix * viewPos;

    // Transform normal to view space
    mat3 normalMatrix = mat3(transpose(inverse(u_ViewMatrix)));
    v_Normal = normalize(normalMatrix * a_Normal);

    // Pass intensity to fragment shader
    v_Intensity = a_InstanceIntensity;
}
