/**
 * UrineParticleSystem.cpp
 */

#include "UrineParticleSystem.hpp"

#include <algorithm>

static const GLfloat vertex_data[] = {
  -0.5f, -0.5f, 0.0f,
  0.5f, -0.5f, 0.0f,
  -0.5f, 0.5f, 0.0f,
  0.5f, 0.5f, 0.0f,
};

UrineParticleSystem::UrineParticleSystem(unsigned int max_particles, const glm::vec3 &position)
  : ParticleSystem(max_particles, position)
{
  _position_data.resize(_max_particles);

  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  // Generate vertex buffer
  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

  // Generate buffer of particle positions
  glGenBuffers(1, &_position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, _position_buffer);
  glBufferData(GL_ARRAY_BUFFER, _max_particles * 3 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
  glVertexAttribDivisor(0, 0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
  glVertexAttribDivisor(1, 1);

  glBindVertexArray(0);
}

void UrineParticleSystem::Update(float delta_time)
{
  unsigned int remaining_particles = _live_particles;
  unsigned int particle_index = 0;

  // Update physics
  for (int i = 0; i < _live_particles; i++)
  {
    ParticlePtr p = _particles[i];

    p->life -= delta_time;
    if (p->life > 0.0f)
    {
      // Determine acceleration
      glm::vec3 accel{ 0.0f };
      accel = physics::gravity;
      accel += _force / _mass;

      // Integrate velocity and position
      p->velocity += accel * delta_time;
      p->position += p->velocity * delta_time;

      // Update particle position buffer data
      _position_data[particle_index] = p->position;

      particle_index++;
    }
    else
    {
      remaining_particles--;
    }

    // Update number of live particles
    _live_particles = remaining_particles;
  }

  // Update buffer data
  glBindBuffer(GL_ARRAY_BUFFER, _position_buffer);
  glBufferData(GL_ARRAY_BUFFER, _max_particles * 3 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, _live_particles * sizeof(GLfloat) * 3, _position_data.data());
}

void UrineParticleSystem::Draw()
{
  glBindVertexArray(_vao);

  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 1, _live_particles);

  glBindVertexArray(0);
}

glm::vec3 UrineParticleSystem::origin() const
{
  return _origin;
}

glm::vec3 UrineParticleSystem::velocity() const
{
  return _velocity;
}

glm::vec3 UrineParticleSystem::force() const
{
  return _force;
}

glm::vec2 UrineParticleSystem::particle_size() const
{
  return _size;
}

float UrineParticleSystem::lifespan() const
{
  return _lifespan;
}

void UrineParticleSystem::set_origin(const glm::vec3 & position)
{
  _origin = position;
}

void UrineParticleSystem::set_velocity(const glm::vec3 & velocity)
{
  _velocity = velocity;
}

void UrineParticleSystem::set_force(const glm::vec3 & force)
{
  _force = force;
}

void UrineParticleSystem::set_size(float x, float y)
{
  _size.x = x;
  _size.y = y;
}

void UrineParticleSystem::set_texture(const char * path)
{
  _texture.id = LoadTextureFromFile(path, "./Resources/Textures");
  _texture.type = TextureType::DIFFUSE;
  _texture.path = std::string(path);
}

void UrineParticleSystem::set_lifespan(float lifespan)
{
  _lifespan = lifespan;
}

void UrineParticleSystem::CreateParticle(unsigned int index)
{
  // Random floating point number between 0.8f to 1.0f
  float random = (static_cast<float>(rand() % 20) / 100.0f) + 0.8f;

  ParticlePtr p = _particles[index];
  p->life     = _lifespan;
  p->position = _origin;
  p->velocity = random * _velocity;

  _live_particles++;
}
