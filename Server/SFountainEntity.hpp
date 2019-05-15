#pragma once

#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"

class SFountainEntity : public SBaseEntity
{
public:
	SFountainEntity(
		glm::vec3 pos,
		glm::vec3 forward)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_FOUNTAIN;

		_state->pos = pos;
		_state->forward = forward;

		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;
		_state->width = 4.4f;
		_state->height = 2.0f;
		_state->depth = 4.4f;
	};
	~SFountainEntity() {};
};

