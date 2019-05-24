#pragma once

#include "IdGenerator.hpp"
#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"

#define PUDDLE_MAX_WIDTH 2.0f
#define PUDDLE_MIN_WIDTH 0.5f
#define PUDDLE_DURATION_MSEC 30000

class SPuddleEntity : public SBaseEntity
{
public:
	SPuddleEntity(glm::vec3 pos)
	{
		// Allocate a state struct and initialize. Modify as necessary for more
		// sane defaults
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();

		// Position from ctor
		_state->type = ENTITY_PUDDLE;
		_state->pos = pos;

		// Capsule collider
		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;

		// Tighter bounding spheres for the bones
		_state->width = PUDDLE_MAX_WIDTH;
		_state->depth = PUDDLE_MAX_WIDTH;
		_state->height = 0.25f;

		// Non-solid object
		_state->isSolid = false;
		_state->transparency = 0.60f;

		hasChanged = true;

		// Expires after 30 seconds
		registerTimer(PUDDLE_DURATION_MSEC, [&]()
			{
				_state->isDestroyed = true;
				hasChanged = true;
			});
	};
	~SPuddleEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events)
	{
		updateTimers();

		// Shrink the puddle
		const float diff = PUDDLE_MAX_WIDTH - PUDDLE_MIN_WIDTH;
		const float durationSecs = PUDDLE_DURATION_MSEC / 1000;
		const float totalTicks = TICKS_PER_SEC * durationSecs;
		const float change = diff / totalTicks; 
		_state->width -= change;
		_state->depth -= change;
		_state->scale.x -= change / 2;
		_state->scale.z -= change / 2;
		hasChanged = true;
	}
};
