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

__kernel void init_particles_cube(__global particle* particles, unsigned int cubeSize) {
	int id = get_global_id(0);

	// Get grid position in the cube using modulus and division
	// Cube root of particle count to divide equally
	int cubeLength = (int)pow(get_global_size(0), 1.0/3.0);
	int xIndex = id % cubeLength;
	int yIndex = (id / cubeLength) % cubeLength;
	int zIndex = id / (cubeLength * cubeLength);

	// Scale grid position to fit inside the cube size
	particles[id].pos.x = (xIndex / (float)cubeLength) * cubeSize - cubeSize / 2.0f;
	particles[id].pos.y = (yIndex / (float)cubeLength) * cubeSize - cubeSize / 2.0f;
	particles[id].pos.z = (zIndex / (float)cubeLength) * cubeSize - cubeSize / 2.0f;
	particles[id].pos_prev = particles[id].pos;

	// Initialize velocity to zero
	particles[id].velocity.x = 0.0f;
	particles[id].velocity.y = 0.0f;
	particles[id].velocity.z = 0.0f;

	// Initialize white particles
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
