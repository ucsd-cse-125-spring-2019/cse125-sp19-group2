#pragma once

#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"
#include "EmptyCollider.hpp"

#define LAUNCHING_VELOCITY 2.0f

class SPlungerEntity : public SBaseEntity
{
public:
	bool launching;

	SPlungerEntity(glm::vec3 pos, glm::vec3 forward)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_PLUNGER;

		_state->pos = pos;
		_state->forward = forward;

		// Basic capsule collider
		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;

		// Slightly tighter bounding box
		_state->width = 0.3f;
		_state->height = 0.6f;
		_state->depth = 0.3f;

		_state->setSolidity([&](BaseState* entity, BaseState* collidingEntity)
		{
			return (collidingEntity->type == ENTITY_HIT_PLUNGER);
		});

		_state->isStatic = false;

		launching = true;
	};

	~SPlungerEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		if (launching) {
			_state->pos += _state->forward * (LAUNCHING_VELOCITY / TICKS_PER_SEC);
			hasChanged = true;
		}
	}

	void generalHandleCollision(SBaseEntity* entity) override
	{
		_collider = std::make_unique<EmptyCollider>();
		_state->colliderType = COLLIDER_NONE;
		_state->isStatic = true;
		launching = false;

		// TODO: make plunger face to the wall
		if (entity->getState()->colliderType == COLLIDER_AABB) {

		}
		else if (entity->getState()->colliderType == COLLIDER_CAPSULE) {
			_state->forward = glm::normalize(entity->getState()->pos - _state->pos);
		}
	}
};

