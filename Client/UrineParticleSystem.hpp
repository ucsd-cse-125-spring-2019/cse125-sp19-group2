/**
 * UrineParticleSystem.hpp
 */

#pragma once

#include "ParticleSystem.hpp"
#include "Texture.hpp"

class UrineParticleSystem : public ParticleSystem
{
public:
  UrineParticleSystem(unsigned int max_particles, const glm::vec3 &position);

  void Update(float delta_timen) override;
  void Draw() override;

  glm::vec3 origin() const;
  glm::vec3 velocity() const;
  glm::vec3 force() const;
  glm::vec2 particle_size() const;
  float lifespan() const;

  void set_origin(const glm::vec3 &position);
  void set_velocity(const glm::vec3 &velocity);
  void set_force(const glm::vec3 &force);
  void set_size(float x, float y);
  void set_texture(const char *path);
  void set_lifespan(float lifespan);

private:
  // Urine particle properties
  glm::vec3 _velocity; ///< Urine's initial velocity;
  glm::vec2 _size;     ///< Dimensions of a urine particle.
  Texture   _texture;  ///< Urine article texture.
  float     _mass;     ///< Urine particle mass in kg.
  float     _lifespan; ///< Lifespan of a urine particle in seconds.

  // Rendering
  GLuint _vao;
  GLuint _vbo;
  GLuint _position_buffer;

  std::vector<glm::vec3> _position_data;

  void CreateParticle(unsigned int index) override;
};