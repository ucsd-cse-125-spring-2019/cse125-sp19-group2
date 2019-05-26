/**
 * ParticleSystem.cpp
 */

#include "ParticleSystem.hpp"

ParticleSystem::ParticleSystem(unsigned int max_particles, const glm::vec3 &origin)
{
	_max_particles = max_particles;
	_origin = origin;
	_particles.resize(_max_particles);

	_live_particles = 0;
}

void ParticleSystem::Reset()
{
	_particles.clear();
	_particles.resize(_max_particles);
}

unsigned int ParticleSystem::Emit(unsigned int num_particles)
{
	while (num_particles && (_live_particles < _max_particles))
	{
		unsigned int index = FindUnusedParticle();
		CreateParticle(index);
		num_particles--;
	}

	return num_particles;
}

unsigned int ParticleSystem::FindUnusedParticle()
{
	for (unsigned int i = _last_particle; i < _max_particles; i++)
	{
		if (_particles[i].life <= 0.0f)
		{
			_last_particle = i;
			return i;
		}
	}

	for (unsigned int i = 0; i < _last_particle; i++)
	{
		if (_particles[i].life <= 0.0f)
		{
			_last_particle = i;
			return i;
		}
	}

	// All particles alive
	_last_particle = 0;
	return 0;
}
