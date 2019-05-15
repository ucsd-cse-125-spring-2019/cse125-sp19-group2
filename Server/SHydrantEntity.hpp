#pragma once

#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"

class SHydrantEntity : public SBaseEntity
{
public:
	SHydrantEntity(
		glm::vec3 pos,
		glm::vec3 forward)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_HYDRANT;

		_state->pos = pos;
		_state->forward = forward;

		// Basic capsule collider
		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;

		// Slightly tighter bounding box
		_state->width = 0.6f;
		_state->height = 0.6f;
		_state->depth = 0.6f;
	};
	~SHydrantEntity() {};
};

