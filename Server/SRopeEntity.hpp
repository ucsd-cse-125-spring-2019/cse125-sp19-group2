#pragma once

#include "SBaseEntity.hpp"
#include "EmptyCollider.hpp"

class SRopeEntity : public SBaseEntity
{
public:
	SRopeEntity()
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_ROPE;

		// Basic capsule collider
		_collider = std::make_unique<EmptyCollider>();
		_state->colliderType = COLLIDER_NONE;
	};

	void updatePoints(glm::vec3 beginPos, glm::vec3 endPos)
	{
		_state->pos = (beginPos + endPos) / 2.0f;
		_state->scale.z = glm::length(endPos - beginPos);
		_state->forward = glm::normalize(endPos - beginPos);
		_state->width = _state->scale.z;
		hasChanged = true;
	}
	~SRopeEntity() {};
};

