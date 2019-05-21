/**
 * UrineParticleSystem.hpp
 */

#pragma once

#include "ParticleSystem.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#include "Shader.hpp"

struct UrineParticle : Particle
{
	float camera_distance = -1.0f;
};

struct UrineParticleSort
{
	bool operator()(const std::shared_ptr<Particle>& a, const std::shared_ptr<Particle>& b)
	{
		return std::dynamic_pointer_cast<UrineParticle>(a)->camera_distance >
			std::dynamic_pointer_cast<UrineParticle>(b)->camera_distance;
	}
} UrineSort;

namespace particle
{
	const char *urine_shader_vert = "./Resources/Shaders/urine.vert";
	const char *urine_shader_frag = "./Resources/Shaders/urine.frag";
}

class UrineParticleSystem : public ParticleSystem
{
public:
	UrineParticleSystem(unsigned int max_particles, const glm::vec3 &origin, const std::shared_ptr<Camera> camera);

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
	typedef std::shared_ptr<UrineParticle> UrineParticlePtr;

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
	std::shared_ptr<Camera> _camera;
	Shader _urine_program;

	std::vector<glm::vec3> _position_data;

	void CreateParticle(unsigned int index) override;
	void SortParticles();
};