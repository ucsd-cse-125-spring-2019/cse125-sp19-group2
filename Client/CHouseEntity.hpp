#pragma once
#include "CBaseEntity.hpp"
#include "Model.hpp"
#include "Shared/Logger.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// As with server, TONS of copied code. Need to refactor static objects
// to initialize all this junk in a parent class.
class CHouseEntity : public CBaseEntity
{
public:
	CHouseEntity(EntityType type) {
		// Allocate member variables

		// House model depends on type. We may want a mapping from type enum to
		// model paths
		switch (type)
		{
		case ENTITY_HOUSE_6X6_A:
		{
			_houseModel = std::make_unique<Model>("./Resources/Models/fountain.fbx");
		}
		}

		_houseShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_houseShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_houseShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_houseShader->CreateProgram();
	}
	~CHouseEntity() {};

	void render(std::unique_ptr<Camera> const& camera) override {
		_houseShader->Use();
		_houseShader->set_uniform("u_projection", camera->projection_matrix());
		_houseShader->set_uniform("u_view", camera->view_matrix());

		// Compute model matrix based on state: t * r * s
		const auto t = glm::translate(glm::mat4(1.0f), _state->pos);
		const auto r = glm::lookAt(glm::vec3(0.0f), _state->forward, _state->up);
		const auto s = glm::scale(glm::mat4(1.0f), _state->scale);

		auto model = t * r * s;
		// Pass model matrix into shader
		_houseShader->set_uniform("u_model", model);
		_houseShader->set_uniform("u_material.shininess", 0.6f);
		_houseShader->set_uniform("u_pointlight.position", glm::vec3(-3.0f, 3.0f, -3.0f));
		_houseShader->set_uniform("u_pointlight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		_houseShader->set_uniform("u_pointlight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		_houseShader->set_uniform("u_pointlight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		_houseShader->set_uniform("u_pointlight.constant", 1.0f);
		_houseShader->set_uniform("u_pointlight.linear", 0.09f);
		_houseShader->set_uniform("u_pointlight.quadratic", 0.032f);
		_houseShader->set_uniform("u_numpointlights", static_cast<GLuint>(1));
		_houseModel->Draw(_houseShader);
	}

	void updateState(std::shared_ptr<BaseState> state) override {
		// Location
		_state->pos = state->pos;

		// Orientation
		_state->forward = state->forward;
		_state->up = state->up;

		// Scale
		_state->scale = state->scale;
	}

	uint32_t getId() override {
		return _state->id;
	}
	
private:
	std::shared_ptr<BaseState> _state;
	std::unique_ptr<Model> _houseModel;
	std::unique_ptr<Shader> _houseShader;
};

