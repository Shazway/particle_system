typedef struct {
	float x, y;
} vec2;

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
} particle;

typedef struct {
	vec3 position;
	vec3 rotationTangent;
	float intensity;
	float radius;
} mass;

__kernel void updateParticles(__global particle *particles, mass m) {
	int id = get_global_id(0);
	const float deltaTime = 0.008f;
	const float decayFactor = 0.995f;
	particle p = particles[id];

	// Save the current position as the previous one for trailing
	particles[id].pos_prev = particles[id].pos;

	vec3 direction;
	direction.x = m.position.x - particles[id].pos.x;
	direction.y = m.position.y - particles[id].pos.y;
	direction.z = m.position.z - particles[id].pos.z;

	// Compute distance from the particle to the center of mass
	float distance = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

	// Normalize the direction vector
	vec3 directionNorm;
	directionNorm.x = direction.x / distance;
	directionNorm.y = direction.y / distance;
	directionNorm.z = direction.z / distance;

	// If the particle is outside the massRadius, apply gravitational attraction
	if (distance > m.radius) {
		// Gravitational force (simplified inverse square law)
		float gravitationalForce = m.intensity / (distance * distance) * 20.0f;

		// Update velocity towards mass center (radial component)
		particles[id].velocity.x += directionNorm.x * gravitationalForce * deltaTime;
		particles[id].velocity.y += directionNorm.y * gravitationalForce * deltaTime;
		particles[id].velocity.z += directionNorm.z * gravitationalForce * deltaTime;
	}
	else
	{
		// Compute cross product to get perpendicular direction for tangential velocity
		vec3 tangentialVelocity;
		tangentialVelocity.x = directionNorm.y * m.rotationTangent.z - directionNorm.z * m.rotationTangent.y;
		tangentialVelocity.y = directionNorm.z * m.rotationTangent.x - directionNorm.x * m.rotationTangent.z;
		tangentialVelocity.z = directionNorm.x * m.rotationTangent.y - directionNorm.y * m.rotationTangent.x;

		// Scale the tangential velocity by some factor
		float tangentialForce = m.intensity / distance;
		tangentialVelocity.x *= tangentialForce * deltaTime;
		tangentialVelocity.y *= tangentialForce * deltaTime;
		tangentialVelocity.z *= tangentialForce * deltaTime;

		// Apply the tangential velocity
		particles[id].velocity.x += tangentialVelocity.x * 2.0f;
		particles[id].velocity.y += tangentialVelocity.y * 2.0f;
		particles[id].velocity.z += tangentialVelocity.z * 2.0f;
	}


	// Slowing down particles so they don't go too far away
	particles[id].velocity.x *= decayFactor;
	particles[id].velocity.y *= decayFactor;
	particles[id].velocity.z *= decayFactor;

	// Update the position based on the updated velocity
	particles[id].pos.x += particles[id].velocity.x * deltaTime;
	particles[id].pos.y += particles[id].velocity.y * deltaTime;
	particles[id].pos.z += particles[id].velocity.z * deltaTime;

	// Normalize distance and avoid division with 0
	float normalizedDist = (distance / m.radius) / 2.0f;
	float totalVelocity = particles[id].velocity.x + particles[id].velocity.y + particles[id].velocity.z;
	float normalizedVelocity =  totalVelocity / 2.0f;
	// Update colors based on distance to the mass point
	particles[id].color.r = clamp(normalizedVelocity - normalizedDist, 0.0f, 1.0f);
	particles[id].color.g = clamp((normalizedDist + normalizedVelocity) * 0.3f, 0.0f, 1.0f);
	particles[id].color.b = clamp(0.5f * normalizedDist, 0.0f, 1.0f);
}
