#pragma once

#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"
#include "SCylinderPlungerEntity.hpp"

class SFountainEntity : public SBaseEntity
{
public:
	SFountainEntity(glm::vec3 pos)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_FOUNTAIN;

		_state->pos = pos;

		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;
		_state->width = 4.4f;
		_state->height = 2.0f;
		_state->depth = 4.4f;

		_children.push_back(std::make_shared<SCylinderPlungerEntity>(pos, glm::vec3(0.8f, 1.0f, 0.8f)));
	};
	~SFountainEntity() {};

	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _children;
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _children;
};

