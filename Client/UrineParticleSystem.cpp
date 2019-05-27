/**
 * UrineParticleSystem.cpp
 */

#include "UrineParticleSystem.hpp"
#include "Shared/Logger.hpp"
#include <algorithm>
#include <glm/gtx/string_cast.hpp>

static const GLfloat vertex_data[] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.0f,
};

UrineParticleSystem::UrineParticleSystem() : UrineParticleSystem(100, glm::vec3(0.0f)) {}

UrineParticleSystem::UrineParticleSystem(unsigned int max_particles, const glm::vec3 &origin)
	: ParticleSystem(max_particles, origin)
{
	_position_data.resize(_max_particles);

	// Setup shader program
	_urine_program = Shader();
	_urine_program.LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/urine.vert");
	_urine_program.LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/urine.frag");
	_urine_program.CreateProgram();

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// Generate vertex buffer for quad
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Generate buffer of instanced particle positions
	glGenBuffers(1, &_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, _position_buffer);
	glBufferData(GL_ARRAY_BUFFER, _max_particles * 3 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glVertexAttribDivisor(1, 1);

	glBindVertexArray(0);
}

void UrineParticleSystem::Update(float delta_time)
{
	_particles_count = 0;

	// Update physics
	for (unsigned int i = 0; i < _max_particles; i++)
	{
		Particle &p = _particles[i];

		if (p.life > 0.0f)
		{
			p.life -= delta_time;
			if (p.life > 0.0f)
			{
				// Determine acceleration
				glm::vec3 accel{ 0.0f };
				accel = physics::gravity;
				accel += _force / _mass;

				// Integrate velocity and position
				p.velocity += accel * delta_time;
				p.position += p.velocity * delta_time;

				// Calculate camera distance for sorting
				//p->camera_distance = glm::length(p->position - _camera->position());

				// Update particle position buffer data
				_position_data[_particles_count] = p.position;
			}
			else
			{
				// Particle has died
				_live_particles--;
			}

			_particles_count++;
		}
	}

	// Sort particles
	//SortParticles();

	// Update number of particles and time
	if (_is_urinating)
	{
		_accum_time += delta_time;
		unsigned int new_particles = _rate * _accum_time;
		_accum_time -= static_cast<float>(new_particles) / _rate;

		Emit(new_particles);
	}

	// Update buffer data
	glBindBuffer(GL_ARRAY_BUFFER, _position_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _particles_count * 3 * sizeof(GLfloat), _position_data.data());
}

void UrineParticleSystem::Draw(std::unique_ptr<Camera> const &camera)
{
	_urine_program.Use();

	// Setup urine sprite texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture.id);
	_urine_program.set_uniform("u_urine_sprite", 0);

	// Particle properties
	_urine_program.set_uniform("u_size", _size);

	// Camera
	_urine_program.set_uniform("u_view", camera->view_matrix());
	_urine_program.set_uniform("u_projection", camera->projection_matrix());

	glBindVertexArray(_vao);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, _particles_count);
	glBindVertexArray(0);

	Logger::getInstance()->info("Drawing " + std::to_string(_particles_count) + " particles");
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

bool UrineParticleSystem::is_urinating() const
{
	return _is_urinating;
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
	Logger::getInstance()->info("Loaded urine texture: " + std::string(path));
	_texture.type = TextureType::DIFFUSE;
	_texture.path = std::string(path);

}

void UrineParticleSystem::set_mass(float mass)
{
	_mass = mass;
}

void UrineParticleSystem::set_lifespan(float lifespan)
{
	_lifespan = lifespan;
}

void UrineParticleSystem::set_rate(float rate)
{
	_rate = rate;
}

void UrineParticleSystem::set_is_urinating(bool is_urinating)
{
	_is_urinating = is_urinating;
	if (!is_urinating) _accum_time = 0; // Reset accumulated time if not emitting
}

void UrineParticleSystem::CreateParticle(unsigned int index)
{
	// Random floating point number between 0.8f to 1.0f
	float random = (static_cast<float>(rand() % 20) / 100.0f) + 0.8f;

	Particle &p = _particles[index];
	p.life     = _lifespan;
	p.position = _origin;
	p.velocity = random * _velocity; // Vary the urine particle velocity randomly by 20%

	_live_particles++;
}
/*
void UrineParticleSystem::SortParticles()
{
	std::sort(_particles.begin(), _particles.end(), UrineSort);
}
*/
