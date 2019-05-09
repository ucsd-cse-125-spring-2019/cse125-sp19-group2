#pragma once

#include "IdGenerator.hpp"
#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"
#include "Shared\PlayerState.hpp"

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
				// Overall direction of player; take average of all direction vectors
				glm::vec3 dir = glm::vec3(0);

				for (auto& event : events)
				{
					// We assume player direction change happens before movement
					if (event->type == EVENT_PLAYER_MOVE)
					{
						dir += glm::vec3(event->direction.x, 0, event->direction.y);
					}
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

