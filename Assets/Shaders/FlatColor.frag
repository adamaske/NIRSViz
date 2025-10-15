#version 330 core

// Output to the framebuffer (usually location 0)
layout(location = 0) out vec4 o_Color;

// Uniform provided by the C++ application (e.g., the white or yellow color)
uniform vec4 u_Color = vec4(1.0, 0.7, 0.3, 1.0);

void main()
{
    // Output the flat color
    o_Color = u_Color;
}