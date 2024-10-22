typedef struct {
	float x, y, z;
} vec3;

typedef struct {
	float r, g, b;
} color;

typedef struct {
	vec3 pos;
	vec3 velocity;
	color color;
} particle;

float fract(float value) {
	return value - floor(value);
}

float get_random(int seed) {
	return fract(sin(seed * 12345.6789f) * 98765.4321f);
}

__kernel void init_particles_sphere(__global particle* particles, float radius) {
	int id = get_global_id(0);

	// Get random spherical coordinates
	float theta = acos(2.0f * get_random(id) - 1.0f);  // Latitude (0 to pi)
	float phi = 2.0f * M_PI * get_random(id + 1);      // Longitude (0 to 2pi)
	float r = (float)cbrt(get_random(id + 2)) * radius;  // Radial distance (0 to radius)

	// Convert spherical coordinates to Cartesian coordinates
	particles[id].pos.x = r * sin(theta) * cos(phi);
	particles[id].pos.y = r * cos(theta);
	particles[id].pos.z = r * sin(theta) * sin(phi);

	// Initialize velocity to zero
	particles[id].velocity.x = 0.0f;
	particles[id].velocity.y = 0.0f;
	particles[id].velocity.z = 0.0f;

	// Initialize particle color (white by default)
	particles[id].color.r = 1.0f;
	particles[id].color.g = 1.0f;
	particles[id].color.b = 1.0f;
}