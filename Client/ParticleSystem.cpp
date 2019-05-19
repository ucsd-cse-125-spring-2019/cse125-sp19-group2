/**
 * ParticleSystem.cpp
 */

#include "ParticleSystem.hpp"

#include <algorithm>

ParticleSystem::ParticleSystem()
{
  _particles.resize(kMaxParticles);
  _position_data.resize(kMaxParticles);
}

void ParticleSystem::Reset()
{
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  // Generate buffer of particle positions
  glGenBuffers(1, &_position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, _position_buffer);
  glBufferData(GL_ARRAY_BUFFER, kMaxParticles * 3 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
  glVertexAttribDivisor(0, 1);

  glBindVertexArray(0);
}

void ParticleSystem::Update(float delta_time, const glm::vec3 &camera_position)
{
  unsigned int remaining_particles = _live_particles;
  unsigned int particle_index = 0;

  // Update physics
  for (int i = 0; i < _live_particles; i++)
  {
    Particle p = _particles[i];

    p.life -= delta_time;
    if (p.life > 0.0f)
    {
      // Determine acceleration
      glm::vec3 accel{ 0.0f };
      accel = physics::gravity;

      // Integrate velocity and position
      p.velocity += accel * delta_time;
      p.position += p.velocity * delta_time;

      // Update particle distance from camera for sorting
      p.camera_distance = glm::length(p.position - camera_position);

      // Update particle position buffer data
      _position_data[particle_index] = p.position;

      particle_index++;
    }
    else
    {
      p.is_live = false;
      p.camera_distance = -1.0f;

      remaining_particles--;
    }

    // Update particle p
    _particles[i] = p;

    // Update number of live particles
    _live_particles = remaining_particles;
  }

  // Update buffer data
  glBindBuffer(GL_ARRAY_BUFFER, _position_buffer);
  glBufferData(GL_ARRAY_BUFFER, kMaxParticles * 3 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, _live_particles * sizeof(GLfloat) * 3, _position_data.data());
}

void ParticleSystem::Draw()
{
  glBindVertexArray(_vao);

  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 1, _live_particles);

  glBindVertexArray(0);
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

void ParticleSystem::set_particle_texture(const char * path)
{
  _particle_texture.id = LoadTextureFromFile(path, "./Resources/Textures");
  _particle_texture.type = TextureType::DIFFUSE;
  _particle_texture.path = std::string(path);
}

void ParticleSystem::set_lifespan(float lifespan)
{
  _lifespan = lifespan;
}

void ParticleSystem::set_rate(unsigned int rate)
{
  _rate = rate;
}

void ParticleSystem::SortParticles()
{
  std::sort(_particles.begin, _particles.end);
}
