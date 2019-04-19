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
		_state->normal = glm::vec3(0, 0, 1);
		_state->up = glm::vec3(0, 1, 0);

		// Default scale is 1
		_state->scale = glm::vec3(1);

		// TODO: load player model here and calculate width, height and depth for
		// collision detection.

		// As of now, just use the same velocity for all players. In the future
		// we might want some arguments in the constructor to determine whether
		// this is a dog or a dog catcher.
		_velocity = 0.1;

		// Players are not static
		_isStatic = false;
	};
	~SPlayerEntity() {};

	void update(
			QuadTree * gameMap,
			std::vector<std::shared_ptr<GameEvent>> events)
	{
		// TODO: respond to events
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
	double _velocity;
};

