#pragma once

#include "IdGenerator.hpp"
#include "SBaseEntity.hpp"

class SPlayerEntity : public SBaseEntity
{
public:
	SPlayerEntity(uint32_t playerId)
	{
		// Allocate a state struct and initialize. Modify as necessary for more
		// sane defaults
		_state = std::make_shared<BaseState>();

		// ID and type
		_state->id = playerId;
		_state->type = ENTITY_PLAYER;

		// At origin by default
		_state->pos = glm::vec3(0);

		// Looking forward (along Z axis)
		_state->forward = glm::vec3(0, 0, 1);
		_state->up = glm::vec3(0, 1, 0);

		// Default scale is 1
		_state->scale = glm::vec3(1);

		// TODO: Set bounding box parameters (width, height, depth, etc)

		_state->isDestroyed = false;

		// As of now, just use the same velocity for all players. In the future
		// we might want some arguments in the constructor to determine whether
		// this is a dog or a dog catcher.
		_velocity = 0.1f;

		// Players are not static
		_isStatic = false;
		_hasChanged = false;
	};
	~SPlayerEntity() {};

	void update(
			QuadTree * gameMap,
			std::vector<std::shared_ptr<GameEvent>> events)
	{
		_hasChanged = false;

		// Only change attributes of this object if not static
		if (!_isStatic)
		{
			// Filter events for this player only
			std::vector<std::shared_ptr<GameEvent>> filteredEvents;
			std::copy_if(events.begin(), events.end(), std::back_inserter(filteredEvents), [&](std::shared_ptr<GameEvent> i)
				{
					return i->playerId == _state->id;
				});

			// Sort by event type
			std::sort(filteredEvents.begin(), filteredEvents.end(),
				[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
				{
					return a->type < b->type;
				});

			// Remove duplicate events
			filteredEvents.erase(std::unique(filteredEvents.begin(), filteredEvents.end(),
				[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
				{
					return a->type == b->type;
				}), filteredEvents.end());

			// If any events left, process them
			if (filteredEvents.size())
			{
				Logger::getInstance()->debug("Received " + std::to_string(filteredEvents.size()) + " events");

				// Overall direction of player
				glm::vec3 dir = glm::vec3(0);

				for (auto& event : filteredEvents)
				{
					switch (event->type)
					{
					case EVENT_PLAYER_MOVE_FORWARD:
						dir += glm::vec3(0, 0, 1);
						break;
					case EVENT_PLAYER_MOVE_BACKWARD:
						dir += glm::vec3(0, 0, -1);
						break;
					case EVENT_PLAYER_MOVE_LEFT:
						dir += glm::vec3(-1, 0, 0);
						break;
					case EVENT_PLAYER_MOVE_RIGHT:
						dir += glm::vec3(1, 0, 0);
						break;
					default:
						break;
					}
				}

				// Update forward vector with unit direction only if it was modified
				if (dir != glm::vec3(0))
				{
					_hasChanged = true;
					_state->forward = dir / glm::length(dir);

					// Move player by (direction * velocity) / ticks_per_sec
					_state->pos = _state->pos + ((_state->forward * _velocity) / (float)TICKS_PER_SEC);
				}
			}
		}
	}

	std::shared_ptr<BaseState> getState(bool ignoreUpdateStatus = false)
	{
		if (ignoreUpdateStatus || _hasChanged)
		{
			return _state;
		}
		
		return NULL;
	}

private:
	// TODO: change this to PlayerState or some such
	std::shared_ptr<BaseState> _state;

	// Player movement velocity in units/second
	float _velocity;
};

