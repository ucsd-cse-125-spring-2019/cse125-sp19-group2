#include "FloorManager.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

std::vector<std::vector<FloorType>> FloorManager::_floorMap;
std::unique_ptr<Model> FloorManager::_floorModel;
std::unique_ptr<Shader> FloorManager::_shader;

FloorManager::FloorManager()
{
}

FloorManager & FloorManager::getInstance()
{
	static FloorManager floorManager;
	if (FloorManager::_floorModel == nullptr) {
		FloorManager::_floorModel = std::make_unique<Model>("./Resources/Models/floor_tile.fbx");
		FloorManager::_shader = std::make_unique<Shader>();
		FloorManager::_shader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/collider.vert");
		FloorManager::_shader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/collider.frag");
		FloorManager::_shader->CreateProgram();
	}

	return floorManager;
}

void FloorManager::update(std::shared_ptr<BaseState> const & state)
{
}

void FloorManager::render(std::unique_ptr<Camera> const & camera)
{
	// Setup shader
	FloorManager::_shader->Use();
	FloorManager::_shader->set_uniform("u_projection", camera->projection_matrix());
	FloorManager::_shader->set_uniform("u_view", camera->view_matrix());

	// Compute model matrix based on state: t * s
	auto mainFloorModel = glm::scale(glm::mat4(1.0f), glm::vec3(MAP_WIDTH, 1, MAP_WIDTH));
}
