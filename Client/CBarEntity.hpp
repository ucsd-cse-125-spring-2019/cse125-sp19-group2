#pragma once
#include "CBaseEntity.hpp"
#include "Model.hpp"

#define BOTTOM_HEIGHT 0.1

class CBarEntity : public CBaseEntity
{
public:
	CBarEntity()
	{
		// Allocate member variables
		_objectModel = std::make_unique<Model>("./Resources/Models/bar.fbx");
		_bottomModel = std::make_unique<Model>("./Resources/Models/bar_bottom.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/wall.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};
	~CBarEntity() {};

	void setUniforms(std::unique_ptr<Camera> const& camera) override
	{
		CBaseEntity::setUniforms(camera);

		// Walls need scale
		const auto s = glm::scale(glm::mat4(1.0f), _state->scale);
		_objectShader->set_uniform("u_scale", s);
	}

	void render(std::unique_ptr<Camera> const &camera) override
	{
		CBaseEntity::render(camera);

		auto t = glm::translate(glm::mat4(1.0f), _state->pos);
		const auto r = glm::lookAt(glm::vec3(0.0f), _state->forward, _state->up);
		auto bottomScale = _state->scale;
		bottomScale.y = BOTTOM_HEIGHT;
		auto s = glm::scale(glm::mat4(1.0f), bottomScale);

		auto model = t * r * s;
		_objectShader->set_uniform("u_model", model);
		_bottomModel->render(_objectShader);

		t = glm::translate(glm::mat4(1.0f), _state->pos + glm::vec3(0, _state->scale.y - BOTTOM_HEIGHT, 0));
		model = t * r * s;
		_objectShader->set_uniform("u_model", model);
		_bottomModel->render(_objectShader);
	}

private:
	std::unique_ptr<Drawable> _bottomModel;
};
