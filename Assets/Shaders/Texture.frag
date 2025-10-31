#version 330 core

layout(location = 0) out vec4 f_Color;
layout(location = 1) out int color2;

in vec2 v_TexCoord;

uniform sampler2D u_Texture; // The texture bound in C++ (slot 0 by default)
uniform vec4 u_Color;        // Optional: A global tint color (default to {1.0, 1.0, 1.0, 1.0})

uniform int u_ID = -1;
uniform int u_UseColor = -1;

void main()
{
    vec4 textureColor = texture(u_Texture, v_TexCoord);
    
    vec4 tint = mix(vec4(1.0), u_Color, u_UseColor);
    f_Color = textureColor * tint;
    
    color2 = u_ID;
}