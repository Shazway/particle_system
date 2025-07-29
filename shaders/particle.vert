#version 430 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_pos_prev;

uniform mat4 viewMatrix;
out vec3 frag_color;

out VS_OUT {
	vec3 pos_curr;
	vec3 color;
	vec3 pos_prev;
} vs_out;

void main()
{
	vs_out.pos_curr = pos_curr;
	vs_out.color = color;
	vs_out.pos_prev = pos_prev;
}
