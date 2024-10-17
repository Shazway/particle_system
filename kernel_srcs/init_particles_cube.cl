typedef struct {
	float x, y, z;
} vec3;

typedef struct {
	vec3 pos;
	vec3 velocity;
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

	// Initialize velocity to zero
	particles[id].velocity.x = 0.0f;
	particles[id].velocity.y = 0.0f;
	particles[id].velocity.z = 0.0f;
}
