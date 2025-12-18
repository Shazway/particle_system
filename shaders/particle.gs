#version 430 core

layout(points) in;
layout(line_strip, max_vertices = 64) out;

layout(std430, binding = 0) readonly buffer ParticleBuffer {
	float particles[];
};

in VS_OUT {
	vec3 pos_curr;
	vec3 color;
	vec3 pos_prev;
} vs_out[];

out vec4 fragColor;

uniform mat4 u_viewProj;
uniform bool u_trailMode;
uniform int  u_trailSamples;
uniform int  u_particleStride; // in floats
uniform int  u_trailOffset;    // in floats
uniform int  u_trailHeadOffset;// in floats

vec3 loadVec3(int base)
{
	return vec3(particles[base], particles[base + 1], particles[base + 2]);
}

void main()
{
	vec3	posCurr = vs_out[0].pos_curr;
	vec3	posPrev = vs_out[0].pos_prev;
	vec3	col = vs_out[0].color;

	// Fast path: legacy short line
	if (!u_trailMode)
	{
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
			gl_Position = u_viewProj * vec4(posPrev, 1.0);
			fragColor = vec4(col, 0.4);
			EmitVertex();

			gl_Position = u_viewProj * vec4(posCurr, 1.0);
			fragColor = vec4(col, 1.0);
			EmitVertex();

			EndPrimitive();
		}
		return;
	}

	// Trailing mode: fetch packed particle data to draw a fading line strip
	int stride = max(u_particleStride, 1);
	int base = gl_PrimitiveIDIn * stride;
	int samples = clamp(u_trailSamples, 1, 63); // leave room for the final vertex

	// Head points to the next slot to be written, so it also marks the oldest sample
	int head = clamp(int(particles[base + u_trailHeadOffset] + 0.5), 0, samples - 1);

	for (int i = 0; i < samples; ++i)
	{
		int idx = (head + i) % samples;
		int offset = base + u_trailOffset + (idx * 3);
		vec3 trailPos = loadVec3(offset);
		float alpha = float(i) / float(samples);

		gl_Position = u_viewProj * vec4(trailPos, 1.0);
		fragColor = vec4(col, alpha * 0.8);
		EmitVertex();
	}

	// Final segment for the current position
	gl_Position = u_viewProj * vec4(posCurr, 1.0);
	fragColor = vec4(col, 1.0);
	EmitVertex();

	EndPrimitive();
}
