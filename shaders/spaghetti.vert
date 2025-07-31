#version 430 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_pos_prev;

uniform mat4 u_viewProj;

out vec3 v_color;

void main()
{
	gl_Position = u_viewProj * vec4(in_pos, 1.0);
	v_color = in_color;
}
