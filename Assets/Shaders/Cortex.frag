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
    // Assumes minVal is negative and maxVal is positive (e.g., -0.05 to 0.05)
    float strength = clamp(rawStrength, minVal, maxVal);

    // Define the colors
    vec3 coldColor = vec3(0.0, 0.0, 1.0); // Blue (for minVal/most negative)
    vec3 centerColor = vec3(0.5, 0.5, 0.5); // Gray (for 0.0)
    vec3 warmColor = vec3(1.0, 0.0, 0.0); // Red (for maxVal/most positive)
    
    // --- Step 2: Determine if the value is negative or positive ---

    if (strength < 0.0) {
        // Cold side (Negative): Interpolate between Blue (minVal) and Gray (0.0)
        
        // a. Normalize the negative range [minVal, 0.0] to [0.0, 1.0].
        // The denominator is -minVal (or abs(minVal)), as minVal is negative.
        // If minVal = -0.05, and strength = -0.025:
        // normalized = (-0.025 - (-0.05)) / (-(-0.05)) = 0.025 / 0.05 = 0.5
        float normalizedNeg = (strength - minVal) / (-minVal);

        // b. Use mix to interpolate: normalizedNeg=0.0 gives Blue (coldColor); 
        //    normalizedNeg=1.0 gives Gray (centerColor).
        return mix(coldColor, centerColor, normalizedNeg);
        
    } else if (strength > 0.0) {
        // Warm side (Positive): Interpolate between Gray (0.0) and Red (maxVal)
        
        // a. Normalize the positive range [0.0, maxVal] to [0.0, 1.0].
        // If maxVal = 0.05, and strength = 0.025:
        // normalized = 0.025 / 0.05 = 0.5
        float normalizedPos = strength / maxVal;

        // b. Use mix to interpolate: normalizedPos=0.0 gives Gray (centerColor); 
        //    normalizedPos=1.0 gives Red (warmColor).
        return mix(centerColor, warmColor, normalizedPos);
        
    } else {
        // Exactly 0.0: Center color
        return centerColor;
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
    
    float specularStrength = 1.0; // Increased for a better look
    vec3 viewDir = normalize(u_ViewPosition - v_WorldPosition); // Calculate view direction
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * u_LightColor;
    
    vec3 light_influence = ambient + diffuse + specular;

    // --- Core Fix ---
    // 1. Get the 'raw' intensity of the color map contribution
    float rayColorIntensity = length(accumulatedRayColor);
    
    // 2. Determine how much the color map should influence the final color.
    // Use the raw object color as a fallback.
    vec3 baseColor = u_ObjectColor;

    // 3. Modulate the base color using the accumulated ray color
    // Use a mix to smoothly blend between the original u_ObjectColor and the ray color.
    // accumulatedRayColor already contains its own 'alpha' through the use of 'alpha' in the loop.
    // Normalize accumulatedRayColor to avoid oversaturation when using it as a direct color.
    
    // Calculate a mixing factor (e.g., based on the color intensity)
    // You might need to experiment with a constant multiplier (e.g., 1.0 or 2.0)
    // to control how quickly the colormap takes over.
    float mixFactor = min(rayColorIntensity, 1.0); 

    // Mix the original object color with the accumulated ray color.
    // The accumulatedRayColor is currently *already* multiplied by 'alpha' from the falloff.
    // To use it as the new object color, we should normalize the accumulated Ray Color by its effective "coverage"
    // to get a clean color value for modulation.
    
    // A simpler, more direct approach for modulation:
    vec3 modulatedObjectColor = mix(u_ObjectColor, accumulatedRayColor, mixFactor);

    // 4. Apply the lighting model to the modulated color.
    // Only the diffuse and ambient components should be affected by the object's color.
    // Specular light should remain white (or u_LightColor) for a white highlight.
    vec3 lit_diffuse_ambient = (ambient + diffuse) * modulatedObjectColor;
    vec3 lit_specular = specular; // Specular is added on top
    
    vec3 final_color = lit_diffuse_ambient + lit_specular;
    
    // Optional: Re-clamp to prevent overly bright pixels, use a higher value for HDR effect
    final_color = min(final_color, vec3(1.0)); // Clamp to 1.0, or higher like 1.5 if you want
                                                 // your colors to 'pop' and allow for a little
                                                 // of the previous oversaturation effect.

    FragColor = vec4(final_color, u_Opacity);
   //vec3 light_influence = ambient + diffuse + specular;
   //vec3 lit_base_color = light_influence * u_ObjectColor; 
   //vec3 final_color = lit_base_color + accumulatedRayColor;
   //
   //final_color = min(final_color, vec3(1.5)); 
   //
   //FragColor = vec4(final_color, u_Opacity);
}
   