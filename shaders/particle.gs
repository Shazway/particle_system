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

void main()
{
	vec3	posCurr = vs_out[0].pos_curr;
	vec3	posPrev = vs_out[0].pos_prev;
	vec3	col = vs_out[0].color;

	float	dist = distance(posCurr, posPrev);
	const float eps = 1e-4;

	if (dist < eps)
	{
		// tiny line that looks like a dot
		vec3	offset = vec3(0.0, 0.01, 0.0);
		float	alphaDot = 0.6;

		gl_Position = u_viewProj * vec4(posCurr - offset, 1.0);
		fragColor = vec4(col, alphaDot);
		EmitVertex();

		gl_Position = u_viewProj * vec4(posCurr + offset, 1.0);
		fragColor = vec4(col, alphaDot);
		EmitVertex();

		EndPrimitive();
	}
	else
	{
		// trail
		gl_Position = u_viewProj * vec4(posPrev, 1.0);
		fragColor = vec4(col, 0.4);
		EmitVertex();

		gl_Position = u_viewProj * vec4(posCurr, 1.0);
		fragColor = vec4(col, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}
