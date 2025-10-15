#version 330 core

// Input from Vertex Shader
in vec3 v_Normal;

// Output color for the framebuffer
out vec4 color;

// Uniforms for Lighting
uniform vec3 u_LightDirection; // Direction *to* the light source (normalized)
uniform vec3 u_LightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 u_ObjectColor = vec3(0.8, 0.8, 0.8);

void main()
{
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(u_LightDirection); // L is now in View Space, matching N

    float diff = max(dot(N, L), 0.0);
    // 4. Calculate the diffuse color
    vec3 diffuse = u_LightColor * u_ObjectColor * diff;

    // Optional: Add a small ambient term to light up the dark side
    vec3 ambient = u_ObjectColor * 0.1; // 10% ambient light

    // 5. Final color is Ambient + Diffuse
    vec3 finalColor = ambient + diffuse;

    color = vec4(finalColor, 1.0);
}