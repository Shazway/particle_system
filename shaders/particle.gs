#version 430 core

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in VS_OUT {
	vec3 pos_curr;
	vec3 color;
	vec3 pos_prev;
} vs_out[];

out vec4 fragColor;

uniform mat4 u_viewProj;

void main() {
	// Here particle is stopped, so it has to be drawn as a point
	if (distance(vs_out[0].pos_curr, vs_out[0].pos_prev) == 0.0)
	{
		gl_Position = u_viewProj * vec4(vs_out[0].pos_curr, 1.0);
		fragColor = vec4(vs_out[0].color, 1.0);
		EmitVertex();
		EndPrimitive();
	}
	// Particle has new and old positions so it can be rendered as line
	else
	{
		gl_Position = u_viewProj * vec4(vs_out[0].pos_prev, 1.0);
		fragColor = vec4(vs_out[0].color, 0.4);
		EmitVertex();

		gl_Position = u_viewProj * vec4(vs_out[0].pos_curr, 1.0);
		fragColor = vec4(vs_out[0].color, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}
