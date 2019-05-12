/**
 * ParticleSystem.hpp
 */

#pragma once

#include "Texture.hpp"

class ParticleSystem
{
public:
  ParticleSystem();

  glm::vec2 particle_size() const;
  float lifespan() const;
  unsigned int rate() const;

  void set_particle_size(float x, float y);
  void set_lifespan(float lifespan);
  void set_rate(unsigned int rate);

private:
  glm::vec2 _particle_size;  ///< Dimensions of a particle.
  Texture _particle_texture; ///< Particle texture.
  float _lifespan;           ///< Lifespan of a particle in seconds
  unsigned int _rate;        ///< Rate of emission in particles per second.


};