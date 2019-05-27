#pragma once

#include "CBaseEntity.hpp"
#include "Model.hpp"

class CPlungerEntity : public CBaseEntity
{
public:
	CPlungerEntity() {
		_objectModel = std::make_unique<Model>("./Resources/Models/plunger.fbx");
		plungerShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		plungerShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		plungerShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		plungerShader->CreateProgram();
	};
	~CPlungerEntity() {};

	virtual void updateState(std::shared_ptr<BaseState> state) override
	{
		// Base update first
		CBaseEntity::updateState(state);

		// Invert z
		_state->forward.z = -_state->forward.z;
	}

	virtual void setUniforms(std::unique_ptr<Camera> const &camera) override
	{
		plungerShader->set_uniform("u_projection", camera->projection_matrix());
		plungerShader->set_uniform("u_view", camera->view_matrix());

		// Setting tranparency
		plungerShader->set_uniform("u_transparency", _state->transparency * _alpha);

		// Compute model matrix based on state: t * r * s
		const auto t = glm::translate(glm::mat4(1.0f), _state->pos);
		const auto r = glm::lookAt(glm::vec3(0.0f), _state->forward, _state->up);
		const auto s = glm::scale(glm::mat4(1.0f), _state->scale);

		auto model = t * r * s;

		// Pass model matrix into shader
		plungerShader->set_uniform("u_model", model);
		plungerShader->set_uniform("u_dirlight.direction", glm::vec3(0.0f, -1.0f, -0.4f));
		plungerShader->set_uniform("u_dirlight.ambient", glm::vec3(0.2f, 0.2f, 0.3f));
		plungerShader->set_uniform("u_dirlight.diffuse", glm::vec3(0.8f, 0.8f, 0.9f));
		plungerShader->set_uniform("u_numdirlights", static_cast<GLuint>(1));
	}

	virtual void render(std::unique_ptr<Camera> const &camera) override
	{
		plungerShader->Use();
		setUniforms(camera);
		_objectModel->render(plungerShader);
	}

	static std::unique_ptr<Shader> plungerShader;
};
