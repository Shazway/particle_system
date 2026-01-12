#define TRAIL_SAMPLES 16

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
	vec3 pos_prev;
	vec3 trail[TRAIL_SAMPLES];
	float trail_timer;
	float trail_head;
	float life;
	float max_life;
	uint seed;
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
	particles[id].pos_prev = particles[id].pos;

	// Initialize velocity to zero
	particles[id].velocity.x = 0.0f;
	particles[id].velocity.y = 0.0f;
	particles[id].velocity.z = 0.0f;

	// Initialize particle color (white by default)
	particles[id].color.r = 1.0f;
	particles[id].color.g = 1.0f;
	particles[id].color.b = 1.0f;

	for (int i = 0; i < TRAIL_SAMPLES; ++i) {
		particles[id].trail[i] = particles[id].pos;
	}
	particles[id].trail_timer = 0.0f;
	particles[id].trail_head = 0.0f;
	particles[id].life = 0.0f;
	particles[id].max_life = 0.0f;
	particles[id].seed = (uint)(id * 747796405u + 2891336453u);
}
