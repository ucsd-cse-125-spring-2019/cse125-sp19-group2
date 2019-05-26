#pragma once

#include "SBaseEntity.hpp"
#include "IdGenerator.hpp"
#include "AABBCollider.hpp"
#include "SBoxPlungerEntity.hpp"

class SHouseEntity : public SBaseEntity
{
public:
	SHouseEntity(
		EntityType houseType,
		glm::vec3 pos,
		glm::vec3 scale)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();

		// Ctor args
		_state->type = houseType;
		_state->pos = pos;

		// Base collider info on scale
		_collider = std::make_unique<AABBCollider>(_state.get());
		_state->colliderType = COLLIDER_AABB;
		_state->width = scale.x;
		_state->height = scale.y;
		_state->depth = scale.z;

		_children.push_back(std::make_shared<SBoxPlungerEntity>(pos, glm::vec3(scale.x - 0.65f, scale.y, scale.z - 0.65f)));
	};
	~SHouseEntity() {};

	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _children;
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _children;
};
