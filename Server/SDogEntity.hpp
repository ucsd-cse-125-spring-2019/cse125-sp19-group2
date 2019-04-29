#pragma once

#include "SPlayerEntity.hpp"

class SDogEntity : public SPlayerEntity
{
public:
	SDogEntity(uint32_t playerId) : SPlayerEntity(playerId)
	{
		_state->type = ENTITY_DOG;

		_state->scale = glm::vec3(0.5f);

		// Collider stuff
		_state->width = 0.25f;
		_state->height = 0.25f;
		_state->depth = 0.25f;

		_velocity = 1;
	};

	~SDogEntity() {};
};

