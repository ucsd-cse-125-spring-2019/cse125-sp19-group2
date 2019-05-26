/**
 * ParticleSystem.hpp
 */

#pragma once

#include "Physics.hpp"
#include "Camera.hpp"
#include <vector>
#include <memory>

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec2 size;
	float mass;
	float life;
	float camera_distance = -1.0f;
};

class ParticleSystem
{
public:
	ParticleSystem(unsigned int max_particles, const glm::vec3 &origin);

	virtual void Update(float delta_time) = 0;
	virtual void Draw(std::unique_ptr<Camera> const &camera) = 0;

	virtual void Reset();
	virtual unsigned int Emit(unsigned int num_particles);

protected:
	// Overall system properties
	glm::vec3 _origin;            ///< Position of particle system.
	glm::vec3 _force;             ///< Force acting on system.
	unsigned int _max_particles;  ///< Maximum number of particles.
	unsigned int _live_particles; ///< Total number of active particles.
	unsigned int _last_particle;

	float _accum_time; ///< Time since last particel was emitted;
	float _rate;       ///< Particles per second.

	std::vector<Particle> _particles;

	virtual void CreateParticle(unsigned int index) = 0;
	virtual unsigned int FindUnusedParticle();
};