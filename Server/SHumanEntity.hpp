#pragma once

#include "SPlayerEntity.hpp"
#include "Shared/HumanState.hpp"

class SHumanEntity : public SPlayerEntity
{
public:
	SHumanEntity(uint32_t playerId)
	{
		_state = std::make_shared<HumanState>();

		// Parent initialization
		SPlayerEntity::initState();
		_state->id = playerId;
		_state->type = ENTITY_HUMAN;

		// Collider stuff
		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;
		_state->width = 0.9f;
		_state->height = 2.0f;
		_state->depth = 0.9f;

		_velocity = 4.8f;

		// Human-specific stuff
		auto humanState = std::static_pointer_cast<HumanState>(_state);
		humanState->currentAnimation = ANIMATION_HUMAN_IDLE;
	};

	~SHumanEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		auto humanState = std::static_pointer_cast<HumanState>(_state);

		// Save old position
		glm::vec3 oldPos = _state->pos;

		// Update and check for changes
		SPlayerEntity::update(events);

		// Set running/not running based on position
		if (_state->pos != oldPos)
		{
			humanState->currentAnimation = ANIMATION_HUMAN_RUNNING;
		}

		// Check for stop event
		for (auto& event : events)
		{
			if (event->type == EVENT_PLAYER_STOP)
			{
				humanState->currentAnimation = ANIMATION_HUMAN_IDLE;
				hasChanged = true;
				break;
			}
		}

		// TODO: net throwing animation
	}

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

