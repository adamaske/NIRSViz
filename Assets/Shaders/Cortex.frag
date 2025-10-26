#version 330 core

in vec3 v_WorldPosition; 
in vec3 v_WorldNormal;

in vec3 LightPos;  

// Output color
out vec4 FragColor;

// --- Uniforms from C++ ---
uniform vec3 u_LightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 u_ObjectColor = vec3(0.8, 0.8, 0.8);
uniform float u_Opacity = 1.0; 

uniform sampler1D u_HitDataTexture; // Our 1D texture for hit data
uniform int u_NumHits;              // Actual number of active hits
const int MAX_HITS_IN_TEXTURE = 256; // Must match C++ MAX_HITS

uniform float u_FalloffPower;
uniform float u_DecayPower;
uniform float u_GlobalHitRadius; 
uniform vec3 u_ViewPosition;      // NEW: Camera/View Position (needed for Specular in World Space)
uniform vec3 u_LightPos;          // Renamed to match Phong convention (was LightPos input)

uniform float u_StrengthMin; // New minimum strength threshold
uniform float u_StrengthMax; // New maximum strength threshold

vec3 colormap(float rawStrength, float minVal, float maxVal) {
    // 1. Clamp the raw strength to the user-defined range
    float strength = clamp(rawStrength, minVal, maxVal);
    
    // 2. Normalize the strength to the [0.0, 1.0] range
    // This scales the value between minVal (0.0) and maxVal (1.0)
    float normalizedStrength = (strength - minVal) / (maxVal - minVal);
    
    // 3. Map the normalized value to the color spectrum (Blue -> Green -> Red)
    vec3 lowColor = vec3(0.0, 0.0, 1.0); // Blue (at normalizedStrength = 0.0)
    vec3 midColor = vec3(0.0, 1.0, 0.0); // Green (at normalizedStrength = 0.5)
    vec3 highColor = vec3(1.0, 0.0, 0.0); // Red (at normalizedStrength = 1.0)
    
    if (normalizedStrength < 0.5) {
        // Interpolate between Blue (0.0) and Green (0.5)
        // normalizedStrength * 2.0 scales the [0.0, 0.5] range to [0.0, 1.0]
        return mix(lowColor, midColor, normalizedStrength * 2.0);
    } else {
        // Interpolate between Green (0.5) and Red (1.0)
        // (normalizedStrength - 0.5) * 2.0 scales the [0.5, 1.0] range to [0.0, 1.0]
        return mix(midColor, highColor, (normalizedStrength - 0.5) * 2.0);
    }
}

float calculateFalloff(float distance, float radius, float power) {
    if (distance > radius) return 0.0;
    float normalizedDist = distance / radius;
    float decayRate = u_DecayPower * power; // Adjust the multiplier (4.0) to fine-tune the visible range
    float alpha = exp(-normalizedDist * decayRate);
    return alpha;
}

void main() {
    vec3 accumulatedRayColor = vec3(0.0);
    
    float texelWidth = 1.0 / float(MAX_HITS_IN_TEXTURE);
    for (int i = 0; i < u_NumHits; ++i) {
        float uCoord = (float(i) + 0.5) * texelWidth;
        vec4 hitData = texture(u_HitDataTexture, uCoord);
        
        vec3 hitPosition = hitData.xyz; // 
        float strength = hitData.w;
        float radius = u_GlobalHitRadius; // Using global radius
        
        float distance = length(v_WorldPosition - hitPosition);
        float alpha = calculateFalloff(distance, radius, u_FalloffPower);
        
        if (alpha > 0.0) {
            vec3 hitColor = colormap(strength, u_StrengthMin, u_StrengthMax);
            // Accumulate the color contribution
            accumulatedRayColor += hitColor * alpha;
        }
    }
    
    vec3 norm = normalize(v_WorldNormal); // Use v_WorldNormal
  
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_LightColor;
    
    vec3 lightDir = normalize(u_LightPos - v_WorldPosition); // Use v_WorldPosition
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;
    
    float specularStrength = 0.5; // Increased for a better look
    vec3 viewDir = normalize(u_ViewPosition - v_WorldPosition); // Calculate view direction
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * u_LightColor;
    
    
    vec3 light_influence = ambient + diffuse + specular;
    vec3 lit_base_color = light_influence * u_ObjectColor; 
    vec3 final_color = lit_base_color + accumulatedRayColor;
   
    final_color = min(final_color, vec3(1.5)); 

    FragColor = vec4(final_color, u_Opacity);
}
   