#pragma once
#include "IdGenerator.hpp"
#include "AABBCollider.hpp"
#include "SBaseEntity.hpp"
#include "Shared/GateState.hpp"

class SGateEntity : public SBaseEntity
{
public:
	SGateEntity(
		glm::vec3 pos,
		glm::vec3 scale)
	{
		// Allocate a state struct and initialize. Modify as necessary for more
		// sane defaults
		_state = std::make_shared<GateState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_GATE;

		// Ctor parameters
		_state->pos = pos;
		_state->scale = scale;

		// Basic AABB, based on scale
		_collider = std::make_unique<AABBCollider>(_state.get());
		_state->colliderType = COLLIDER_AABB;
		_state->width = scale.x;
		_state->height = scale.y;
		_state->depth = scale.z;

		auto gateState = std::static_pointer_cast<GateState>(_state);
		gateState->jailIndex = 0; // TODO: change this to specific jail
		gateState->liftRate = 0;
	};
	~SGateEntity() {};
};