#pragma once

#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"
#include "SCylinderPlungerEntity.hpp"

class SHydrantEntity : public SBaseEntity
{
public:
	SHydrantEntity(glm::vec3 pos)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_HYDRANT;

		_state->pos = pos;

		// Basic capsule collider
		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;

		// Slightly tighter bounding box
		_state->width = 0.6f;
		_state->height = 0.6f;
		_state->depth = 0.6f;

		_children.push_back(std::make_shared<SCylinderPlungerEntity>(pos, glm::vec3(0.5f, 0.6f, 0.5f)));
	};
	~SHydrantEntity() {};

	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _children;
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _children;
};

