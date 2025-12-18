#version 430 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_pos_prev;

out VS_OUT {
	vec3 pos_curr;
	vec3 color;
	vec3 pos_prev;
} vs_out;

void main()
{
	vs_out.pos_curr = in_pos;
	vs_out.color = in_color;
	vs_out.pos_prev = in_pos_prev;
	// Pass-through position for completeness; geometry shader handles transform
	gl_Position = vec4(in_pos, 1.0);
}
