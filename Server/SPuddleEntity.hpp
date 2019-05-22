#pragma once

#include "IdGenerator.hpp"
#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"

class SPuddleEntity : public SBaseEntity
{
public:
	SPuddleEntity(glm::vec3 pos)
	{
		// Allocate a state struct and initialize. Modify as necessary for more
		// sane defaults
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();

		// Position from ctor
		_state->type = ENTITY_PUDDLE;
		_state->pos = pos;

		// Capsule collider
		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;

		// Tighter bounding spheres for the bones
		_state->width = 0.6f;
		_state->depth = 0.6f;
		_state->height = 0.6f;

		// Non-solid object
		_state->isSolid = false;

		hasChanged = true;
	};
	~SPuddleEntity() {};
};
