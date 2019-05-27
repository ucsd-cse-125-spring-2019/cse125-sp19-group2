#pragma once

#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"

class STrapEntity : public SBaseEntity
{
public:
	STrapEntity(glm::vec3 pos)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();

		_state->type = ENTITY_TRAP;
		_state->pos = pos;

		// Capsule collider
		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;

		_state->width = 2.5f;
		_state->depth = 2.5f;
		_state->height = 0.6f;

		// Non-solid object
		_state->isSolid = false;

		hasChanged = true;
	}
	~STrapEntity() {};
};
