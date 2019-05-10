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
		dogState->runStamina = MAX_DOG_STAMINA;
	};

	~SDogEntity() {};

	bool isCaught = false;

	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		auto dogState = std::static_pointer_cast<DogState>(_state);

		// Save old position
		glm::vec3 oldPos = _state->pos;

		// Filter for non-movement events
		auto filteredEvents = std::vector<std::shared_ptr<GameEvent>>();
		for (auto& event : events)
		{
			if (event->type != EVENT_PLAYER_MOVE)
			{
				filteredEvents.push_back(event);
			}
		}

		// Remove duplicates. This should probably be moved to the EventManager,
		// because the human will have to do the exact same thing
		filteredEvents.erase(std::unique(filteredEvents.begin(), filteredEvents.end(),
			[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
		{
			return a->type == b->type;
		}), filteredEvents.end());

		// If any events left, process them
		if (filteredEvents.size())
		{
			for (auto& event : filteredEvents)
			{
				switch (event->type)
				{
				case EVENT_PLAYER_RUN:
					// TODO: find out when dog stops running. Will have the same problem
					// when tick rate is higher than FPS unless client generates event
					// when dog stops running.
					if (dogState->runStamina > 0)
					{
						_velocity = RUN_VELOCITY;
					}
					dogState->runStamina -= 1 / TICKS_PER_SEC;
					
					if (dogState->runStamina < 0)
					{
						dogState->runStamina = 0;
					}

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

		// Check for stop event
		for (auto& event : events)
		{
			if (event->type == EVENT_PLAYER_STOP)
			{
				dogState->currentAnimation = ANIMATION_DOG_IDLE;
				hasChanged = true;
				break;
			}
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
			// Refill dog stamina
			dogState->runStamina = MAX_DOG_STAMINA;
			hasChanged = true;

			// Remove dog bone
			entity->getState()->isDestroyed = true;
			entity->hasChanged = true;
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

