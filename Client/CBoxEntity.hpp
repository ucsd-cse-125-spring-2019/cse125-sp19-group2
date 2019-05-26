#pragma once
#include "CBaseEntity.hpp"
#include "Model.hpp"

class CBoxEntity : public CBaseEntity
{
public:
	CBoxEntity()
	{
		// TODO: possibly can erase all loading since this will be an invisible box in the future
		// Allocate member variables
		_objectModel = std::make_unique<Model>("./Resources/Models/fence.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/wall.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};
	~CBoxEntity() {};

	void setUniforms(std::unique_ptr<Camera> const& camera) override
	{
		CBaseEntity::setUniforms(camera);

		// Walls need scale
		const auto s = glm::scale(glm::mat4(1.0f), _state->scale);
		_objectShader->set_uniform("u_scale", s);

        // Setting tranparency
		_objectShader->set_uniform("u_transparency", _state->transparency);
	}

    virtual float getAlpha() const override {
	    return 1.0f;
	}

	void render(std::unique_ptr<Camera> const& camera) override
	{

	}
};
