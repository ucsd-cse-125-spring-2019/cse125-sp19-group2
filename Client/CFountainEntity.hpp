#pragma once

#include "CBaseEntity.hpp"
#include "Model.hpp"

class CFountainEntity : public CBaseEntity
{
public:
	CFountainEntity() {
		_objectModel = std::make_unique<Model>("./Resources/Models/fountain.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};
	~CFountainEntity() {};
};

