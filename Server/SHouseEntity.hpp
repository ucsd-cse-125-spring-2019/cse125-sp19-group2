#pragma once

#include "SBaseEntity.hpp"
#include "IdGenerator.hpp"
#include "AABBCollider.hpp"

// This is almost exactly the same as the SBoxEntity class...
// I think at some point we can just slap all static objects that
// don't need an update function and use an AABB into the same class,
// like SStaticBoxEntity. Otherwise this will get unmanageable, fast.
class SHouseEntity : public SBaseEntity
{
public:
	SHouseEntity(
		EntityType houseType,
		glm::vec3 pos,
		glm::vec3 forward,
		glm::vec3 scale)
	{
		_state = std::make_shared<BaseState>();
		_state->id = IdGenerator::getInstance()->getNextId();
		_state->type = houseType;
		_state->pos = pos;
		_state->forward = forward;
		_state->up = glm::vec3(0, 1, 0);
		_state->scale = glm::vec3(1, 1, 1);

		// Base collider info on scale
		_state->width = scale.x;
		_state->height = scale.y;
		_state->depth = scale.z;
		_state->colliderType = COLLIDER_AABB;

		// Basic AABB collider
		_collider = std::make_unique<AABBCollider>(_state.get());

		// Server state info
		_state->isDestroyed = false;
		_state->isStatic = true;
		_state->isSolid = true;

		hasChanged = false;
	};
	~SHouseEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		// Update does nothing for houses atm
	}

	std::shared_ptr<BaseState> getState() override
	{
		return _state;
	}

private:
	std::shared_ptr<BaseState> _state;
};
