#pragma once

#include "SBaseEntity.hpp"
#include "AABBCollider.hpp"

class SDogHouseEntity : public SBaseEntity
{
public:
	SDogHouseEntity(
		glm::vec3 pos,
		glm::vec3 forward)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_DOGHOUSE;

		// Ctor parameters
		_state->pos = pos;
		_state->forward = forward;

		// Basic AABB. This will probably have to change to at least
		// three child AABB colliders, so the dog can actually go
		// inside the doghouse.
		_collider = std::make_unique<AABBCollider>(_state.get());
		_state->colliderType = COLLIDER_AABB;
		_state->width = 1.2f;
		_state->height = 1.2f;
		_state->depth = 1.2f;
	};
	~SDogHouseEntity() {};
};

