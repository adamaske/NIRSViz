#version 330 core
layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform float u_Opacity = 1.0;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    FragColor = vec4(texColor.rgb, texColor.a * u_Opacity);
}