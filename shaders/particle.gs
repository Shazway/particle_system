#version 430 core

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in VS_OUT {
	vec3 pos_curr;
	vec3 pos_prev;
	vec3 color;
} gs_in[];

out vec4 fragColor;

uniform mat4 u_viewProj;

void main() {
	// Previous point (tail)
	gl_Position = u_viewProj * vec4(gs_in[0].pos_prev, 1.0);
	fragColor = vec4(gs_in[0].color, 0.0); // Transparent tail
	EmitVertex();

	// Current point (head)
	gl_Position = u_viewProj * vec4(gs_in[0].pos_curr, 1.0);
	fragColor = vec4(gs_in[0].color, 1.0); // Solid head
	EmitVertex();

	EndPrimitive();
}
