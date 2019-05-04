#pragma once

#include "SPlayerEntity.hpp"

class SHumanEntity : public SPlayerEntity
{
public:
	SHumanEntity(uint32_t playerId) : SPlayerEntity(playerId)
	{
		_state->type = ENTITY_HUMAN;

		_state->scale = glm::vec3(1);

		// Collider stuff
		_state->width = 0.9f;
		_state->height = 0.9f;
		_state->depth = 0.9f;

		_velocity = 4.8f;
	};

	~SHumanEntity() {};
};

