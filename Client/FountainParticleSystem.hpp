/**
 * FountainParticleSystem.hpp
 */

#pragma once

#include "ParticleSystem.hpp"
#include "Texture.hpp"
#include "Shader.hpp"

class FountainParticleSystem : public ParticleSystem
{
public:
	FountainParticleSystem();
	FountainParticleSystem(unsigned int max_particles, const glm::vec3 &origin);

	void Update(float delta_time) override;
	void Draw(std::unique_ptr<Camera> const &camera) override;

	glm::vec3 origin() const;
	glm::vec3 force() const;
	glm::vec2 particle_size() const;
	float lifespan() const;

	void set_origin(const glm::vec3 &position);
	void set_force(const glm::vec3 &force);
	void set_size(float x, float y);
	void set_texture(const char *path);
	void set_mass(float mass);
	void set_speed(float speed);
	void set_angle(float angle);
	void set_lifespan(float lifespan);
	void set_rate(float rate);

private:
	// Water particle properties
	glm::vec2 _size;     ///< Dimensions of a water particle.
	Texture   _texture;  ///< Water article texture.
	float     _mass;     ///< Water particle mass in kg.
	float     _speed;    ///< Speed of water particle in m/s.
	float     _angle;    ///< Angle of spray relative to ground.
	float     _lifespan; ///< Lifespan of a water particle in seconds.

	// System properties
	unsigned int _particles_count;

	// Rendering
	GLuint _vao;
	GLuint _vbo;
	GLuint _position_buffer;
	Shader _fountain_program;

	std::vector<glm::vec3> _position_data;

	void CreateParticle(unsigned int index) override;
};