#pragma once

#include "IdGenerator.hpp"
#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"

class SBoneEntity : public SBaseEntity
{
public:
	SBoneEntity(glm::vec3 pos) 
	{
		// Allocate a state struct and initialize. Modify as necessary for more
		// sane defaults
		_state = std::make_shared<BaseState>();

		// ID and type
		_state->id = IdGenerator::getInstance()->getNextId();
		_state->type = ENTITY_BONE;

		// At origin by default
		_state->pos = pos;

		// Looking forward (along Z axis)
		_state->forward = glm::vec3(0, 0, 1);
		_state->up = glm::vec3(0, 1, 0);

		// Standard scale
		_state->scale = glm::vec3(1);

		// Collider stuff
		_state->colliderType = COLLIDER_CAPSULE;

		// Tighter bounding spheres for the bones
		_state->width = 0.6f;
		_state->height = 0.6f;
		_state->depth = 0.6f;

		_state->isDestroyed = false;
		_state->isStatic = true;
		_state->isSolid = false;

		// collider to track collision info idk
		_collider = std::make_unique<CapsuleCollider>(_state.get());

		hasChanged = false;
	};

	// Update function, called every tick
	void update(std::vector<std::shared_ptr<GameEvent>> events) override 
	{
		
	}

	void handleCollision(std::shared_ptr<SBaseEntity> entity) override
	{
	}

	// All server objects must have a state to send to the client.
	std::shared_ptr<BaseState> getState() override
	{
		return _state;
	}

private:
	std::shared_ptr<BaseState> _state;
};