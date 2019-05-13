#pragma once

#include "CBaseEntity.hpp"

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
			_objectModel = std::make_unique<Model>("./Resources/Models/house_6x6_A.fbx");
		}
		}

		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	}
	~CHouseEntity() {};
};

