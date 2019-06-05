#pragma once

#include "CBaseEntity.hpp"
#include "Model.hpp"

class CGrassEntity : public CBaseEntity
{
public:
	CGrassEntity(glm::vec3 pos) {
		_state = std::make_shared<BaseState>();
		// Base defaults
		this->initState();
		_state->type = ENTITY_GRASS;

		_state->pos = pos;
		_objectModel = std::make_unique<Model>("./Resources/Models/grass.fbx");
		_objectShader = std::make_unique<Shader>();


		// Slightly tighter bounding box
		_state->width = 0.6f;
		_state->height = 0.6f;
		_state->depth = 0.6f;
		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};
	~CGrassEntity() {};
	void initState()
	{

		// At origin, looking forward, with 1x1x1 scale
		_state->pos = glm::vec3(0);
		_state->forward = glm::vec3(0, 0, -1);
		_state->up = glm::vec3(0, 1, 0);
		_state->scale = glm::vec3(1);

		// Set default transparency to 1
		_state->transparency = 1.0f;

		// Defaults to static solid object
		_state->isDestroyed = false;
		_state->isStatic = true;
		_state->isSolid = true;
		_state->isVisible = true;
	}
};
