#pragma once

#include <memory>
#include "IdGenerator.hpp"
#include "AABBCollider.hpp"
#include "SBaseEntity.hpp"
#include "Shared/BaseState.hpp"

class SBoxEntity : public SBaseEntity
{
public:
	SBoxEntity(
		glm::vec3 pos,
		glm::vec3 scale)
	{
		// Allocate a state struct and initialize. Modify as necessary for more
		// sane defaults
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_BOX;

		// Ctor parameters
		_state->pos = pos;
		_state->scale = scale;

		// Basic AABB, based on scale
		_collider = std::make_unique<AABBCollider>(_state.get());
		_state->colliderType = COLLIDER_AABB;
		_state->width = scale.x;
		_state->height = scale.y;
		_state->depth = scale.z;
	};
	~SBoxEntity() {};
};
