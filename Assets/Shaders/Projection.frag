#version 330 core
out vec4 FragColor;

in vec3 Normal; 
in vec3 FragPos;
in vec3 LightPos;   

uniform vec3 u_LightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 u_ObjectColor = vec3(0.9, 0.35, 0.25);
uniform float u_Opacity = 1.0; 

in float vActivityLevel;
uniform float u_StrengthMin; // Minimum Activity Level threshold
uniform float u_StrengthMax; // Maximum Activity Level threshold

vec3 calculateActivityColor(float activity) {
    vec3 lowColor    = vec3(0.0, 0.0, 1.0); 
    vec3 neutralColor = vec3(0.4, 0.4, 0.4); 
    vec3 highColor   = vec3(1.0, 0.0, 0.0); 

    float t_unclamped = (activity - u_StrengthMin) / (u_StrengthMax - u_StrengthMin);
    float t = clamp(t_unclamped, 0.0, 1.0);
    
    vec3 color;
    float t_scaled = t * 2.0;

    if (t_scaled <= 1.0) {
        color = mix(lowColor, neutralColor, t_scaled);
    } else {
        color = mix(neutralColor, highColor, t_scaled - 1.0);
    }
    return color;
}

void main()
{
    // --- A. Calculate Base Activity Color ---
    vec3 activityColor = calculateActivityColor(vActivityLevel);
    
     // ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * u_LightColor;    
    
     // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;
    
    // specular
    float specularStrength = 0.2;
    vec3 viewDir = normalize(-FragPos); // the viewer is always at (0,0,0) in view-space, so viewDir is (0,0,0) - Position => -Position
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * u_LightColor; 
    
    vec3 result = (ambient + diffuse + specular) * activityColor;
    FragColor = vec4(result, u_Opacity);
}