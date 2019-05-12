#pragma once
#include "CBaseEntity.hpp"
#include "Model.hpp"

class CBoxEntity : public CBaseEntity
{
public:
	CBoxEntity() {
		// Allocate member variables
		_objectModel = std::make_unique<Model>("./Resources/Models/wall.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/wall.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};
	~CBoxEntity() {};
};

