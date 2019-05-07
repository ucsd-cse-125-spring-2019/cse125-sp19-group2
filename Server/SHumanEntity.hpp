#pragma once

#include "SPlayerEntity.hpp"
#include "Shared/HumanState.hpp"

class SHumanEntity : public SPlayerEntity
{
public:
	SHumanEntity(uint32_t playerId) : SPlayerEntity(playerId)
	{
		_state = std::make_shared<HumanState>();

		// Parent initialization
		SPlayerEntity::initState(playerId);

		_state->type = ENTITY_HUMAN;

		_state->scale = glm::vec3(1);

		// Collider stuff
		_state->width = 0.9f;
		_state->height = 0.9f;
		_state->depth = 0.9f;

		_velocity = 4.8f;

		// Human-specific stuff
		auto humanState = std::static_pointer_cast<HumanState>(_state);
	};

	~SHumanEntity() {};

	void handleCollision(std::shared_ptr<SBaseEntity> entity)
	{
		// Cast for player-specific stuff
		auto humanState = std::static_pointer_cast<HumanState>(_state);

		// Dog getting caught is not handled by the human
		if (entity->getState()->type != ENTITY_DOG)
		{
			SBaseEntity::handleCollision(entity);
		}
	}
};

