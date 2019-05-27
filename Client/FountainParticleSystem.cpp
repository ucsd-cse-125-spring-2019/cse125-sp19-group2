/**
 * WaterParticleSystem.cpp
 */

#include "FountainParticleSystem.hpp"
#include "Shared/Logger.hpp"
#include <algorithm>
#include <glm/gtx/string_cast.hpp>

static const GLfloat vertex_data[] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.0f,
};

FountainParticleSystem::FountainParticleSystem() : FountainParticleSystem(1000, glm::vec3(0.0f)) {}

FountainParticleSystem::FountainParticleSystem(unsigned int max_particles, const glm::vec3 &origin)
	: ParticleSystem(max_particles, origin)
{
	_position_data.resize(_max_particles);

	// Setup shader program
	_fountain_program = Shader();
	_fountain_program.LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/urine.vert");
	_fountain_program.LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/urine.frag");
	_fountain_program.CreateProgram();

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

void FountainParticleSystem::Update(float delta_time)
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

	// Update number of particles and time
	_accum_time += delta_time;
	unsigned int new_particles = _rate * _accum_time;
	_accum_time -= static_cast<float>(new_particles) / _rate;

	Emit(new_particles);

	// Update buffer data
	glBindBuffer(GL_ARRAY_BUFFER, _position_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, _particles_count * 3 * sizeof(GLfloat), _position_data.data());
}

void FountainParticleSystem::Draw(std::unique_ptr<Camera> const &camera)
{
	_fountain_program.Use();

	// Setup urine sprite texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _texture.id);
	_fountain_program.set_uniform("u_urine_sprite", 0);

	// Particle properties
	_fountain_program.set_uniform("u_size", _size);

	// Camera
	_fountain_program.set_uniform("u_view", camera->view_matrix());
	_fountain_program.set_uniform("u_projection", camera->projection_matrix());

	glBindVertexArray(_vao);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, _particles_count);
	glBindVertexArray(0);
}

glm::vec3 FountainParticleSystem::origin() const
{
	return _origin;
}

glm::vec3 FountainParticleSystem::force() const
{
	return _force;
}

glm::vec2 FountainParticleSystem::particle_size() const
{
	return _size;
}

float FountainParticleSystem::lifespan() const
{
	return _lifespan;
}

void FountainParticleSystem::set_origin(const glm::vec3 & position)
{
	_origin = position;
}

void FountainParticleSystem::set_force(const glm::vec3 & force)
{
	_force = force;
}

void FountainParticleSystem::set_size(float x, float y)
{
	_size.x = x;
	_size.y = y;
}

void FountainParticleSystem::set_texture(const char * path)
{
	_texture.id = LoadTextureFromFile(path, "./Resources/Textures");
	Logger::getInstance()->info("Loaded urine texture: " + std::string(path));
	_texture.type = TextureType::DIFFUSE;
	_texture.path = std::string(path);

}

void FountainParticleSystem::set_mass(float mass)
{
	_mass = mass;
}

void FountainParticleSystem::set_speed(float speed)
{
	_speed = speed;
}

void FountainParticleSystem::set_angle(float angle)
{
	_angle = angle;
}

void FountainParticleSystem::set_lifespan(float lifespan)
{
	_lifespan = lifespan;
}

void FountainParticleSystem::set_rate(float rate)
{
	_rate = rate;
}

void FountainParticleSystem::CreateParticle(unsigned int index)
{
	// Random angle of velocity
	float random = static_cast<float>(rand() % 360);

	Particle &p = _particles[index];
	p.life = _lifespan;
	p.position = _origin;
	p.velocity =  glm::vec3(glm::rotate(random, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::rotate(-_angle, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::vec4(0.0f, 0.0f, _speed, 0.0f));

	_live_particles++;
}
