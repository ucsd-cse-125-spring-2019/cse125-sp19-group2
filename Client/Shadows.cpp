#include "Shadows.hpp"
#include "EntityManager.hpp"

Shadows::Shadows() : Shadows(1024, 1024, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f))
{
}

Shadows::Shadows(int width, int height, const glm::vec3 &light_pos, const glm::vec3 &light_dir)
{
	_map_width  = width;
	_map_height = height;

	_pass_shadow_map = std::make_unique<FrameBuffer>(_map_width, _map_height);

	_light_camera = std::make_unique<Camera>(true);
	_light_camera->set_position(light_pos);
	_light_camera->set_forward(light_dir);

	_shadow_map_program = std::make_unique<Shader>();
	_shadow_map_program->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/shadow_map.vert");
	_shadow_map_program->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/shadow_map.frag");
}

Shadows::~Shadows()
{
}

void Shadows::process(GLsizei width, GLsizei height)
{
	glViewport(0, 0, _map_width, _map_height);
	_pass_shadow_map->renderScene([this]
	{
		EntityManager::getInstance().render(_light_camera);
	});
	glViewport(0, 0, width, height);
}
