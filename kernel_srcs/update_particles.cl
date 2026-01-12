#define TRAIL_SAMPLES 16
#define TRAIL_INTERVAL 0.07f

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

typedef struct {
	vec3 position;
	vec3 rotationTangent;
	float intensity;
	float radius;
} mass;

typedef struct {
	vec3 position;
	float spawn_radius;
	float push_intensity;
	float push_radius;
	float spawn_speed;
	float life_min;
	float life_max;
	uint enabled;
} emitter;

uint lcg(uint *state)
{
	*state = (*state * 1664525u) + 1013904223u;
	return *state;
}

float rand01(uint *state)
{
	return (float)(lcg(state) & 0x00FFFFFFu) / 16777216.0f;
}

__kernel void updateParticles(__global particle *particles, mass m, emitter e, float deltaTime, uint emitterStart) {
	int id = get_global_id(0);
	// Exponential damping scaled by real deltaTime so it remains frame-rate independent.
	// decayRate is chosen so that exp(-decayRate * (1/60)) ~= 0.995f (old per-frame factor at 60 FPS).
	const float decayRate = 0.30075f;
	const float eps = 0.0001f;

	// Save the current position as the previous one for trailing
	particles[id].pos_prev = particles[id].pos;

	const int isEmitter = (e.enabled != 0u) && id >= (int)emitterStart;
	if (isEmitter) {
		particles[id].life -= deltaTime;
		if (particles[id].life <= 0.0f) {
			uint seed = particles[id].seed ^ (uint)(id * 747796405u + 2891336453u);
			float u = rand01(&seed);
			float v = rand01(&seed);
			float theta = 6.2831853f * u;
			float z = 1.0f - 2.0f * v;
			float xy = sqrt(fmax(0.0f, 1.0f - z * z));
			vec3 dir = {xy * cos(theta), xy * sin(theta), z};
			float spawnScale = pow(rand01(&seed), 0.3333333f) * e.spawn_radius;
			particles[id].pos.x = e.position.x + dir.x * spawnScale;
			particles[id].pos.y = e.position.y + dir.y * spawnScale;
			particles[id].pos.z = e.position.z + dir.z * spawnScale;
			particles[id].pos_prev = particles[id].pos;

			particles[id].velocity.x = dir.x * e.spawn_speed;
			particles[id].velocity.y = dir.y * e.spawn_speed;
			particles[id].velocity.z = dir.z * e.spawn_speed;

			particles[id].max_life = e.life_min + (e.life_max - e.life_min) * rand01(&seed);
			particles[id].life = particles[id].max_life;

			for (int i = 0; i < TRAIL_SAMPLES; ++i) {
				particles[id].trail[i] = particles[id].pos;
			}
			particles[id].trail_timer = 0.0f;
			particles[id].trail_head = 0.0f;

			particles[id].seed = seed;
		}
	}

	vec3 direction;
	direction.x = m.position.x - particles[id].pos.x;
	direction.y = m.position.y - particles[id].pos.y;
	direction.z = m.position.z - particles[id].pos.z;

	// Compute distance from the particle to the center of mass
	float distance = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	float invDist = 1.0f / fmax(distance, eps);

	// Normalize the direction vector
	vec3 directionNorm;
	directionNorm.x = direction.x * invDist;
	directionNorm.y = direction.y * invDist;
	directionNorm.z = direction.z * invDist;

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
		float tangentialForce = m.intensity / fmax(distance, eps);
		tangentialVelocity.x *= tangentialForce * deltaTime;
		tangentialVelocity.y *= tangentialForce * deltaTime;
		tangentialVelocity.z *= tangentialForce * deltaTime;

		// Apply the tangential velocity
		particles[id].velocity.x += tangentialVelocity.x * 2.0f;
		particles[id].velocity.y += tangentialVelocity.y * 2.0f;
		particles[id].velocity.z += tangentialVelocity.z * 2.0f;
	}

	// Emitter repulsion (push)
	if (e.enabled != 0u) {
		vec3 eDir;
		eDir.x = particles[id].pos.x - e.position.x;
		eDir.y = particles[id].pos.y - e.position.y;
		eDir.z = particles[id].pos.z - e.position.z;
		float eDist = sqrt(eDir.x * eDir.x + eDir.y * eDir.y + eDir.z * eDir.z);
		if (eDist > eps && eDist < e.push_radius) {
			float invEDist = 1.0f / eDist;
			float repulse = e.push_intensity / (eDist * eDist + 1.0f);
			particles[id].velocity.x += (eDir.x * invEDist) * repulse * deltaTime;
			particles[id].velocity.y += (eDir.y * invEDist) * repulse * deltaTime;
			particles[id].velocity.z += (eDir.z * invEDist) * repulse * deltaTime;
		}
	}

	// Slowing down particles so they don't go too far away
	const float damping = exp(-decayRate * deltaTime);
	particles[id].velocity.x *= damping;
	particles[id].velocity.y *= damping;
	particles[id].velocity.z *= damping;

	// Update the position based on the updated velocity
	particles[id].pos.x += particles[id].velocity.x * deltaTime;
	particles[id].pos.y += particles[id].velocity.y * deltaTime;
	particles[id].pos.z += particles[id].velocity.z * deltaTime;

	// Normalize distance and avoid division with 0
	float normalizedDist = (distance / m.radius) / 2.0f;
	float totalVelocity = particles[id].velocity.x + particles[id].velocity.y + particles[id].velocity.z;
	float normalizedVelocity = totalVelocity / 2.0f;

	// Update colors based on distance to the mass point
	particles[id].color.r = clamp(normalizedVelocity - normalizedDist, 0.0f, 1.0f);
	particles[id].color.g = clamp((normalizedDist + normalizedVelocity) * 0.3f, 0.0f, 1.0f);
	particles[id].color.b = clamp(0.5f * normalizedDist, 0.0f, 1.0f);

	if (isEmitter) {
		float lifeRatio = (particles[id].max_life > 0.0f) ? (particles[id].life / particles[id].max_life) : 0.0f;
		lifeRatio = clamp(lifeRatio, 0.0f, 1.0f);
		particles[id].color.r = 1.0f;
		particles[id].color.g = lifeRatio;
		particles[id].color.b = lifeRatio;
	}

	// Trail bookkeeping: sample the path roughly every TRAIL_INTERVAL seconds
	float accumulator = particles[id].trail_timer + deltaTime;
	int head = (int)(particles[id].trail_head + 0.5f);

	while (accumulator >= TRAIL_INTERVAL) {
		particles[id].trail[head] = particles[id].pos;
		head = (head + 1) % TRAIL_SAMPLES;
		accumulator -= TRAIL_INTERVAL;
	}
	particles[id].trail_timer = accumulator;
	particles[id].trail_head = (float)head;
}
