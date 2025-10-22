#version 440 core
uniform vec4 u_PointColor = vec4(1, 1, 1, 1);

uniform float u_PointSize;

out vec4 FragColor;

void main()
{
    // Make point a circle
    vec2 coord = gl_PointCoord * 2.0 - 1.0; // [-1,1] range
    if (dot(coord, coord) > 1.0)
        discard;

    FragColor = u_PointColor; 
}
