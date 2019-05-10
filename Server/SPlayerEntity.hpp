#pragma once

#include "IdGenerator.hpp"
#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"
#include "Shared\PlayerState.hpp"

// Amount of leway when comparing floats
const float FP_EPSILON = 0.00001f;

class SPlayerEntity : public SBaseEntity
{
public:
	SPlayerEntity(uint32_t playerId)
	{
		hasChanged = false;
	};

	// Needs to be in a helper function b/c children create state objectb
	void initState(uint32_t playerId) override
	{
		// ID and type
		_state->id = playerId;

		// At origin by default
		_state->pos = glm::vec3(0);

		// Looking forward (along Z axis)
		_state->forward = glm::vec3(0, 0, 1);
		_state->up = glm::vec3(0, 1, 0);

		// Standard scale
		_state->scale = glm::vec3(1);

		_state->colliderType = COLLIDER_CAPSULE;

		_state->isDestroyed = false;
		_state->isStatic = false;
		_state->isSolid = true;

		// Capsule collider
		_collider = std::make_unique<CapsuleCollider>(_state.get());
	}

	~SPlayerEntity() {};

	virtual void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		// Do nothing if we are set to be destroyed
		if (_state->isDestroyed)
		{
			return;
		}

		hasChanged = false;

		// Only change attributes of this object if not static
		if (!_state->isStatic)
		{
			// If any events left, process them
			if (events.size())
			{
				// Movement
				std::vector<std::shared_ptr<GameEvent>> movementEvents;
				std::copy_if(events.begin(), events.end(), std::back_inserter(movementEvents), [&](std::shared_ptr<GameEvent> i)
				{
					return i->type == EVENT_PLAYER_MOVE;
				});

				// Sort by vector
				std::sort(movementEvents.begin(), movementEvents.end(),
					[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
					{
						if (std::abs(a->direction.x - b->direction.x) < FP_EPSILON)
							return a->direction.x < b->direction.x;
						else
							return a->direction.y < b->direction.y;
					});

				// Remove duplicate vectors
				movementEvents.erase(std::unique(movementEvents.begin(), movementEvents.end(),
					[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
					{
						return a->direction == b->direction;
					}), movementEvents.end());

				// Overall direction of player; take average of all direction vectors
				glm::vec3 dir = glm::vec3(0);

				for (auto& moveEvent : movementEvents)
				{
					dir += glm::vec3(moveEvent->direction.x, 0, moveEvent->direction.y);
				}

				// Update forward vector with unit direction only if it was modified
				if (dir != glm::vec3(0))
				{
					hasChanged = true;
					_state->forward = dir / glm::length(dir);


					// Move player by (direction * velocity) / ticks_per_sec
					auto oldPos = _state->pos;
					_state->pos = _state->pos + ((_state->forward * _velocity) / (float)TICKS_PER_SEC);
				}
			}
		}
	}

	virtual std::shared_ptr<BaseState> getState() override
	{
		return _state;
	}

protected:
	// TODO: change this to PlayerState or some such
	std::shared_ptr<PlayerState> _state;

	// Player movement velocity in units/second
	float _velocity;
};

