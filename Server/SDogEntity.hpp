#pragma once

#include <random>
#include "SPlayerEntity.hpp"
#include "Shared/DogState.hpp"

#define BASE_VELOCITY 5.0f
#define RUN_VELOCITY BASE_VELOCITY * 1.5f;

class SDogEntity : public SPlayerEntity
{
public:
	SDogEntity(uint32_t playerId, std::vector<glm::vec2>* jails) : SPlayerEntity(playerId)
	{
		_state = std::make_shared<DogState>();

		// Parent initialization
		SPlayerEntity::initState(playerId);

		_state->type = ENTITY_DOG;

		_state->scale = glm::vec3(1);

		// Collider stuff
		_state->width = 0.8f;
		_state->height = 0.8f;
		_state->depth = 0.8f;

		_velocity = BASE_VELOCITY;

		_jails = jails;

		// Dog-specific stuff
		auto dogState = std::static_pointer_cast<DogState>(_state);
		dogState->currentAnimation = ANIMATION_DOG_IDLE;
		dogState->runStamina = 10;
	};

	~SDogEntity() {};

	bool isCaught = false;

	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		auto dogState = std::static_pointer_cast<DogState>(_state);

		// Save old position
		glm::vec3 oldPos = _state->pos;

		events.erase(std::unique(events.begin(), events.end(),
			[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
		{
			return a->type == b->type;
		}), events.end());

		// If any events left, process them
		if (events.size())
		{
			for (auto& event : events)
			{
				switch (event->type)
				{
				case EVENT_PLAYER_RUN:
					if (dogState->runStamina > 0)
					{
						_velocity = RUN_VELOCITY;
					}
					dogState->runStamina -= 1;

					break;
				// TODO: event for when player releases running button
				default:
					break;
				}
			}
		}

		// Update and check for changes
		SPlayerEntity::update(events);

		// Set running/not running based on position
		if (_state->pos != oldPos)
		{
			dogState->currentAnimation = ANIMATION_DOG_RUNNING;
		}
		else if (dogState->currentAnimation != ANIMATION_DOG_IDLE)
		{
			dogState->currentAnimation = ANIMATION_DOG_IDLE;
			hasChanged = true;
		}

		// TODO: catching animation
	}

	void handleCollision(std::shared_ptr<SBaseEntity> entity) override
	{
		// Cast for dog-specific stuff
		auto dogState = std::static_pointer_cast<DogState>(_state);

		// Dog getting caught is handled by the dog, not the human
		if (entity->getState()->type == ENTITY_HUMAN)
		{
			isCaught = true;
			
			// Choose a random jail
			unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
			std::shuffle(_jails->begin(), _jails->end(), std::default_random_engine(seed));
			glm::vec2 jailPos = (*_jails)[0];
			getState()->pos = glm::vec3(jailPos.x, 0, jailPos.y);
		}
		else if (entity->getState()->type == ENTITY_BONE)
		{
			dogState->runStamina += 150;
		}
		else
		{
			// Otherwise, handle collisions as usual
			SBaseEntity::handleCollision(entity);
		}
	}

private:
	// List of jails the dog could potentially be sent to
	std::vector<glm::vec2>* _jails;
	int type = 0;
};

