#version 330 core

// Inputs from vertex shader
in vec3 v_FragPos;
in vec3 v_Normal;
in vec3 v_WorldPos;
in float v_Intensity;

// Output color
out vec4 FragColor;

// Uniforms
uniform float u_MinIntensity = 0.0;
uniform float u_MaxIntensity = 1.0;
uniform float u_TransparencyThreshold = 0.01;
uniform int u_Colormap = 0;  // 0 = Viridis, 1 = Jet, 2 = Grayscale
uniform float u_AmbientStrength = 0.3;
uniform float u_SpecularStrength = 0.4;
uniform int u_Shininess = 32;

// Viridis colormap - perceptually uniform and colorblind-friendly
// Maps value [0,1] to RGB color
vec3 viridis(float t) {
    // Viridis color points (polynomial approximation)
    const vec3 c0 = vec3(0.2777273272, 0.0050598636, 0.3340998053);
    const vec3 c1 = vec3(0.1050930431, 1.4043196933, 1.3841755402);
    const vec3 c2 = vec3(-0.3308618287, 0.2145144843, -0.1159376804);
    const vec3 c3 = vec3(-4.6340009845, -5.7996395323, -19.3389353652);
    const vec3 c4 = vec3(6.2281904966, 14.1785116855, 56.6908228291);
    const vec3 c5 = vec3(4.7768116500, -13.7451916020, -65.3532156526);
    const vec3 c6 = vec3(-5.4356460503, 4.6452123522, 26.3124299301);

    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
}

// Jet colormap - classic heat map (blue -> cyan -> yellow -> red)
vec3 jet(float t) {
    float r = clamp(1.5 - abs(4.0 * t - 3.0), 0.0, 1.0);
    float g = clamp(1.5 - abs(4.0 * t - 2.0), 0.0, 1.0);
    float b = clamp(1.5 - abs(4.0 * t - 1.0), 0.0, 1.0);
    return vec3(r, g, b);
}

// Grayscale colormap
vec3 grayscale(float t) {
    return vec3(t, t, t);
}

// Get color from selected colormap
vec3 getHeatMapColor(float normalized_intensity) {
    if (u_Colormap == 0) {
        return viridis(normalized_intensity);
    } else if (u_Colormap == 1) {
        return jet(normalized_intensity);
    } else {
        return grayscale(normalized_intensity);
    }
}

void main()
{
    // Normalize intensity to [0, 1] range
    float normalized_intensity = clamp((v_Intensity - u_MinIntensity) / (u_MaxIntensity - u_MinIntensity), 0.0, 1.0);

    // Get heat map color
    vec3 heat_color = getHeatMapColor(normalized_intensity);

    // Calculate alpha based on intensity (low intensity = more transparent)
    float alpha = smoothstep(u_TransparencyThreshold, u_TransparencyThreshold + 0.1, normalized_intensity);

    // --- Professional Phong Lighting ---
    vec3 normal = normalize(v_Normal);

    // Multiple light sources for better visualization
    vec3 keyLight = normalize(vec3(2.0, 3.0, 2.0));      // Main light (top-right-front)
    vec3 fillLight = normalize(vec3(-1.0, 0.5, 1.0));    // Fill light (left-front)
    vec3 rimLight = normalize(vec3(0.0, -1.0, -1.0));    // Rim light (back-bottom)

    vec3 viewDir = normalize(-v_FragPos);  // Camera at origin in view space

    // Ambient component
    vec3 ambient = u_AmbientStrength * heat_color;

    // Key light (main directional light)
    float keyDiff = max(dot(normal, keyLight), 0.0);
    vec3 keyReflect = reflect(-keyLight, normal);
    float keySpec = pow(max(dot(viewDir, keyReflect), 0.0), u_Shininess);
    vec3 keyContribution = (0.6 * keyDiff + u_SpecularStrength * keySpec) * heat_color;

    // Fill light (softer, lower intensity)
    float fillDiff = max(dot(normal, fillLight), 0.0);
    vec3 fillContribution = 0.3 * fillDiff * heat_color;

    // Rim light (subtle back lighting for depth)
    float rimDiff = max(dot(normal, rimLight), 0.0);
    vec3 rimContribution = 0.2 * rimDiff * heat_color;

    // Edge detection for crisp voxel appearance
    float edgeFactor = abs(dot(viewDir, normal));
    edgeFactor = smoothstep(0.0, 0.2, edgeFactor); // Darken edges slightly

    // Combine all lighting
    vec3 final_color = ambient + keyContribution + fillContribution + rimContribution;
    final_color *= (0.85 + 0.15 * edgeFactor); // Subtle edge darkening

    // Slight desaturation for more professional look
    float luminance = dot(final_color, vec3(0.299, 0.587, 0.114));
    final_color = mix(vec3(luminance), final_color, 0.95);

    // Output with transparency
    FragColor = vec4(final_color, alpha);
}
