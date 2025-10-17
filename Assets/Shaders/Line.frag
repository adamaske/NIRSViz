#version 330 core

out vec4 o_Color;
in vec4 v_Color; 

uniform vec4 u_LineColor;

void main()
{
    o_Color = u_LineColor;
}