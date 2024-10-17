typedef struct {
	float x, y;
} vec2;

typedef struct {
	float x, y, z;
} vec3;

typedef struct {
	vec3 pos;
	vec3 velocity;
} particle;

typedef struct {
	vec3 position;
	float intensity;
	float radius;
} mass;

__kernel void updateParticles(__global particle *particles, mass m) {
	int id = get_global_id(0);
	const float deltaTime = 0.016f;
	particle p = particles[id];

	// Calculate direction vector towards the mass point
	vec3 direction;
	direction.x = m.position.x - p.pos.x;
	direction.y = m.position.y - p.pos.y;
	direction.z = m.position.z - p.pos.z;

	// Calculate the distance from the particle to the mass center
	float distance = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

	// Normalize direction
	if (distance > 0.0f) {
		direction.x /= distance;
		direction.y /= distance;
		direction.z /= distance;
	}

	// Adjust force based on distance and mass radius
	float force;
	if (distance < m.radius) {
		// Gradually reduce force within the radius
		force = (m.intensity / m.radius) * (distance / m.radius);
	} else {
		// Outside the radius, apply inverse-square law
		force = m.intensity / (distance * distance);
	}

	// Apply acceleration to velocity
	p.velocity.x += direction.x * force * deltaTime;
	p.velocity.y += direction.y * force * deltaTime;
	p.velocity.z += direction.z * force * deltaTime;

	// Update position based on velocity
	p.pos.x += p.velocity.x * deltaTime;
	p.pos.y += p.velocity.y * deltaTime;
	p.pos.z += p.velocity.z * deltaTime;

	particles[id] = p;
}
