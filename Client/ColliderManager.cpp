#include "ColliderManager.hpp"
#include "Shared/Logger.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

std::unique_ptr<Model> ColliderManager::_cubeModel;
std::unique_ptr<Model> ColliderManager::_cylinderModel;
std::unique_ptr<Shader> ColliderManager::_shader;

ColliderManager::ColliderManager()
{
}

ColliderManager& ColliderManager::getInstance() {
	static ColliderManager colliderManager;
	// Load models and shader if not loaded
	if (ColliderManager::_cubeModel == nullptr) {
		ColliderManager::_cubeModel = std::make_unique<Model>("./Resources/Models/cube.fbx");
		ColliderManager::_cylinderModel = std::make_unique<Model>("./Resources/Models/cylinder.fbx");
		ColliderManager::_shader = std::make_unique<Shader>();
		ColliderManager::_shader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/collider.vert");
		ColliderManager::_shader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/collider.frag");
		ColliderManager::_shader->CreateProgram();
	}
	
	return colliderManager;
}

void ColliderManager::updateState(std::shared_ptr<BaseState> const& state) {
	// Ignore baseState that don't have collider
	if (state->colliderType == COLLIDER_NONE) {
		return;
	}

	auto result = colliderList.find(state->id);
	// Collision box already created
	if (result != colliderList.end()) {
		result->second->pos = state->pos;
		result->second->scale.x = state->width;
		result->second->scale.y = state->height;
		result->second->scale.z = state->depth;
	}
	// Collision box not found and need to create it
	else {
		std::shared_ptr<colliderInfo> newInfo = std::make_shared<colliderInfo>();
		newInfo->colliderType = state->colliderType;
		newInfo->scale = glm::vec3(state->width, state->height, state->depth);
		newInfo->pos = state->pos;
		colliderList.insert({ state->id, newInfo });
	}
}

void ColliderManager::erase(uint32_t id) {
	auto result = colliderList.find(id);
	if (result != colliderList.end()) {
		colliderList.erase(result);
	}
}

void ColliderManager::render(std::unique_ptr<Camera> const& camera) {

	if (!renderMode) return;

	// Setup shader
	ColliderManager::_shader->Use();
	ColliderManager::_shader->set_uniform("u_projection", camera->projection_matrix());
	ColliderManager::_shader->set_uniform("u_view", camera->view_matrix());

	// render all collider
	std::for_each(colliderList.begin(), colliderList.end(),
		[&camera](std::pair<uint32_t, std::shared_ptr<colliderInfo>> entry)
	{
		std::shared_ptr<colliderInfo> curBox = entry.second;

		// Compute model matrix based on state: t * s
		const auto t = glm::translate(glm::mat4(1.0f), curBox->pos);
		const auto s = glm::scale(glm::mat4(1.0f), curBox->scale);

		auto model = t * s;

		// Pass model matrix into shader
		ColliderManager::_shader->set_uniform("u_model", model);

		switch (curBox->colliderType) {
		case COLLIDER_GATE:
		case COLLIDER_AABB:
			ColliderManager::_cubeModel->getMeshAt(0).DrawLine(ColliderManager::_shader);
			break;
		case COLLIDER_CAPSULE:
			ColliderManager::_cylinderModel->getMeshAt(0).DrawLine(ColliderManager::_shader);
			break;
		}
	});
}

void ColliderManager::clear() {
	colliderList.clear();
}