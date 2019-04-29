#pragma once

#include "SPlayerEntity.hpp"

class SHumanEntity : public SPlayerEntity
{
public:
	SHumanEntity(uint32_t playerId) : SPlayerEntity(playerId)
	{
		_state->type = ENTITY_HUMAN;

		_state->scale = glm::vec3(0.5f);

		// Collider stuff
		_state->width = 0.3f;
		_state->height = 0.3f;
		_state->depth = 0.3f;

		_velocity = 0.9f;
	};

	~SHumanEntity() {};
};

