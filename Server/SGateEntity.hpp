#pragma once
#include "SBaseEntity.hpp"
#include "Shared/GateState.hpp"

class SGateEntity : public SBaseEntity
{
public:
	SGateEntity(
		glm::vec3 pos,
		glm::vec3 scale)
	{
		_state = std::make_shared<GateState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_GATE;

		_state->pos = pos;
		_state->scale = scale;

		// Gate collider
		_collider = std::make_unique<AABBCollider>(_state.get());
		_state->colliderType = COLLIDER_GATE;
		_state->width = scale.x;
		_state->height = scale.y;
		_state->depth = scale.z;

		// Gate-specific stuff
		auto gateState = std::static_pointer_cast<GateState>(_state);
		gateState->isLifting = false;
	};
	~SGateEntity() {};
};