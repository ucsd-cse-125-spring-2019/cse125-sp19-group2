#pragma once

#include "SPlayerEntity.hpp"

class SDogEntity : public SPlayerEntity
{
public:
	SDogEntity(uint32_t playerId) : SPlayerEntity(playerId)
	{
		_state->type = ENTITY_DOG;

		_state->scale = glm::vec3(1);

		// Collider stuff
		_state->width = 0.8f;
		_state->height = 0.8f;
		_state->depth = 0.8f;

		_velocity = 5.0f;
	};

	~SDogEntity() {};

	bool isCaught = false;
};

