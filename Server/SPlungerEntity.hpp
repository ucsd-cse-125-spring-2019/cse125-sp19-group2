#pragma once

#include "SBaseEntity.hpp"
#include "PlungerCollider.hpp"
#include "EmptyCollider.hpp"
#include "Shared/PlungerState.hpp"

#define LAUNCHING_VELOCITY 22.0f

class SPlungerEntity : public SBaseEntity
{
public:
	bool launching;

	SPlungerEntity(glm::vec3 pos, glm::vec3 forward)
	{
		_state = std::make_shared<PlungerState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_PLUNGER;

		_state->pos = pos;
		_state->forward = forward;

		// Basic capsule collider
		_collider = std::make_unique<PlungerCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;

		// Slightly tighter bounding box
		_state->width = 0.35f;
		_state->height = 0.6f;
		_state->depth = 0.35f;

		_state->setSolidity([&](BaseState* entity, BaseState* collidingEntity)
		{
			return (collidingEntity->type == ENTITY_HIT_PLUNGER);
		});

		_state->isStatic = false;

		// Plunger-specific stuff
		auto plungerState = std::static_pointer_cast<PlungerState>(_state);
		plungerState->isStuck = false;

		launching = true;
	};

	~SPlungerEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		if (_state->isDestroyed)
		{
			return;
		}

		if (launching) {
			_state->pos += _state->forward * (LAUNCHING_VELOCITY / TICKS_PER_SEC);
		}
		hasChanged = true;
	}

	void generalHandleCollision(SBaseEntity* entity) override
	{
		if (entity->getState()->type == ENTITY_HIT_PLUNGER) {
			_collider = std::make_unique<EmptyCollider>();
			_state->colliderType = COLLIDER_NONE;
			_state->isStatic = true;
			launching = false;

			// Set plunger as stuck
			auto plungerState = std::static_pointer_cast<PlungerState>(_state);
			plungerState->isStuck = true;

			auto stateB = entity->getState();

			// make plunger face the wall
			if (stateB->colliderType == COLLIDER_AABB) {
				float dists[4];
				dists[0] = (stateB->pos.x - stateB->width / 2) - _state->pos.x; // West
				dists[1] = _state->pos.x - (stateB->pos.x + stateB->width / 2); // East
				dists[2] = _state->pos.z - (stateB->pos.z + stateB->depth / 2); // North
				dists[3] = (stateB->pos.z - stateB->depth / 2) - _state->pos.z; // South
				int minIndex = -1;
				float min = FLT_MAX;

				// Get closest edge
				for (int i = 0; i < 4; i++)
				{
					if (dists[i] > 0)
					{
						minIndex = i;
						break;
					}

					if (dists[i] < min)
					{
						min = dists[i];
						minIndex = i;
					}
				}

				switch (minIndex)
				{
				case 0: // West
					_state->forward = glm::vec3(1, 0, 0);
					break;
				case 1: // East
					_state->forward = glm::vec3(-1, 0, 0);
					break;
				case 2: // North
					_state->forward = glm::vec3(0, 0, -1);
					break;
				case 3: // South
					_state->forward = glm::vec3(0, 0, 1);
					break;
				}

			}
			else if (stateB->colliderType == COLLIDER_CAPSULE) {
				_state->forward = glm::normalize(stateB->pos - _state->pos);
			}
		}
	}
};

