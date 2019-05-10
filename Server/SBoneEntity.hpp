#pragma once
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

		// At origin by default
		_state->pos = pos;

		// Looking forward (along Z axis)
		_state->forward = glm::vec3(0, 0, 1);
		_state->up = glm::vec3(0, 1, 0);

		// Standard scale
		_state->scale = glm::vec3(0.1f);

		_state->colliderType = COLLIDER_CAPSULE;

		_state->isDestroyed = false;
		_state->isStatic = false;

		// collider to track collision info idk
		_collider = std::make_unique<CapsuleCollider>(_state.get());

		hasChanged = false;
	};
private:
	std::shared_ptr<BaseState> _state;
};