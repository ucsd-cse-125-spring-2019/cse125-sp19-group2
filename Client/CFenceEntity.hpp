#pragma once
#include "CBaseEntity.hpp"
#include "Model.hpp"

class CFenceEntity : public CBaseEntity
{
public:
	CFenceEntity()
	{
		// Allocate member variables
		_objectModel = std::make_unique<Model>("./Resources/Models/fence.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/wall.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/wall.frag");
		_objectShader->CreateProgram();
	};
	~CFenceEntity() {};

	void setUniforms(std::unique_ptr<Camera> const& camera) override
	{
		CBaseEntity::setUniforms(camera);

		// Walls need scale
		const auto s = glm::scale(glm::mat4(1.0f), _state->scale);
		_objectShader->set_uniform("u_scale", s);

		// Transparency (don't use alpha)
		_objectShader->set_uniform("u_transparency", _state->transparency);
	}

	float getAlpha() const override {
		return 1.0f;
	}
};
