/**
 * UrineParticleSystem.hpp
 */

#pragma once

#include "ParticleSystem.hpp"
#include "Texture.hpp"
#include "Shader.hpp"

/*
struct UrineParticleSort
{
	bool operator()(const std::shared_ptr<Particle>& a, const std::shared_ptr<Particle>& b)
	{
		return a->camera_distance > b->camera_distance;
	}
} UrineSort;

namespace particle
{
	const char *urine_shader_vert = "./Resources/Shaders/urine.vert";
	const char *urine_shader_frag = "./Resources/Shaders/urine.frag";
}
*/
class UrineParticleSystem : public ParticleSystem
{
public:
	UrineParticleSystem();
	UrineParticleSystem(unsigned int max_particles, const glm::vec3 &origin);

	void Update(float delta_time) override;
	void Draw(std::unique_ptr<Camera> const &camera) override;

	glm::vec3 origin() const;
	glm::vec3 velocity() const;
	glm::vec3 force() const;
	glm::vec2 particle_size() const;
	float lifespan() const;
	bool is_urinating() const;

	void set_origin(const glm::vec3 &position);
	void set_velocity(const glm::vec3 &velocity);
	void set_force(const glm::vec3 &force);
	void set_size(float x, float y);
	void set_texture(const char *path);
	void set_mass(float mass);
	void set_lifespan(float lifespan);
	void set_rate(float rate);
	void set_is_urinating(bool is_urinating);

private:
	// Urine particle properties
	glm::vec3 _velocity; ///< Urine's initial velocity;
	glm::vec2 _size;     ///< Dimensions of a urine particle.
	Texture   _texture;  ///< Urine article texture.
	float     _mass;     ///< Urine particle mass in kg.
	float     _lifespan; ///< Lifespan of a urine particle in seconds.

	// System properties
	unsigned int _particles_count;
	bool _is_urinating; ///< Whether urine particle system should emit particles.

	// Rendering
	GLuint _vao;
	GLuint _vbo;
	GLuint _position_buffer;
	Shader _urine_program;

	std::vector<glm::vec3> _position_data;

	void CreateParticle(unsigned int index) override;
	//void SortParticles();
};