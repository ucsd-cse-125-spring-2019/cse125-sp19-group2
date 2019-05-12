#include "ParticleSystem.hpp"

ParticleSystem::ParticleSystem()
{
}

glm::vec2 ParticleSystem::particle_size() const
{
  return _particle_size;
}

float ParticleSystem::lifespan() const
{
  return _lifespan;
}

unsigned int ParticleSystem::rate() const
{
  return _rate;
}

void ParticleSystem::set_particle_size(float x, float y)
{
  _particle_size.x = x;
  _particle_size.y = y;
}

void ParticleSystem::set_lifespan(float lifespan)
{
  _lifespan = lifespan;
}

void ParticleSystem::set_rate(unsigned int rate)
{
  _rate = rate;
}
