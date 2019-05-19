/**
 * ParticleSystem.hpp
 */

#pragma once

#include "Texture.hpp"
#include "Physics.hpp"
#include <vector>

struct Particle
{
  glm::vec3 position;
  glm::vec3 velocity;
  float life;
  float camera_distance = -1.0f;
  bool is_live = false;

  bool operator<(Particle &p)
  {
    this->camera_distance > p.camera_distance;
  }
};

class ParticleSystem
{
public:
  ParticleSystem();

  void Reset();
  void Update(float delta_time, const glm::vec3 &camera_position);
  void Draw();

  glm::vec2 particle_size() const;
  float lifespan() const;
  unsigned int rate() const;

  void set_particle_size(float x, float y);
  void set_particle_texture(const char *path);
  void set_lifespan(float lifespan);
  void set_rate(unsigned int rate);

private:
  const unsigned int kMaxParticles = 1000;

  // System properties
  glm::vec2 _particle_size;     ///< Dimensions of a particle.
  Texture _particle_texture;    ///< Particle texture.
  float _lifespan;              ///< Lifespan of a particle in seconds
  unsigned int _rate;           ///< Rate of emission in particles per second.
  unsigned int _live_particles; ///< Total number of active particles

  // Physics
  glm::vec3 _direction; ///< Direction of particle emission.
  float _particle_mass; ///< Particle mass in kg.

  // Rendering
  GLuint _vao;
  GLuint _position_buffer;

  std::vector<Particle> _particles;
  std::vector<glm::vec3> _position_data;

  void SortParticles();
};