#pragma once

#include "CBaseEntity.hpp"
#include "Model.hpp"

class CRopeEntity : public CBaseEntity
{
public:
	CRopeEntity() {
		_objectModel = std::make_unique<Model>("./Resources/Models/rope.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};

	virtual void updateState(std::shared_ptr<BaseState> state) override
	{
		// Base update first
		CBaseEntity::updateState(state);

		// Invert z
		_state->forward.z = -_state->forward.z;
	}

	~CRopeEntity() {};
};

