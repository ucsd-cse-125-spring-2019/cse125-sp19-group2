#pragma once

#include "SBoxEntity.hpp"
#include "EmptyCollider.hpp"

#define JAIL_WALL_WIDTH 0.25f

class SJailEntity : public SBaseEntity
{
public:
	SJailEntity(
		glm::vec3 pos,
		glm::vec3 forward,
		glm::vec3 scale)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_JAIL;

		_state->pos = pos;
		_state->forward = forward;
		_state->scale = scale;

		// No collider on the jail object, only on its children
		_state->colliderType = COLLIDER_NONE;
		_collider = std::make_unique<EmptyCollider>();

		// Jails are not rendered at all
		_state->isSolid = false;

		// North wall
		_walls.push_back(std::make_shared<SBoxEntity>(
			glm::vec3(pos.x, pos.y, pos.z + scale.z / 2 - JAIL_WALL_WIDTH / 2),
			glm::vec3(scale.x, scale.y, JAIL_WALL_WIDTH)
		));

		// South wall
		_walls.push_back(std::make_shared<SBoxEntity>(
			glm::vec3(pos.x, pos.y, pos.z - scale.z / 2 + JAIL_WALL_WIDTH / 2),
			glm::vec3(scale.x, scale.y, JAIL_WALL_WIDTH)
		));

		// West wall
		_walls.push_back(std::make_shared<SBoxEntity>(
			glm::vec3(pos.x - scale.x / 2 + JAIL_WALL_WIDTH / 2, pos.y, pos.z),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, scale.z - JAIL_WALL_WIDTH*2)
		));

		// East wall
		_walls.push_back(std::make_shared<SBoxEntity>(
			glm::vec3(pos.x + scale.x / 2 - JAIL_WALL_WIDTH / 2, pos.y, pos.z),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, scale.z - JAIL_WALL_WIDTH*2)
		));
	};
	~SJailEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		// Update does nothing for jails at the moment
	}

	// If walls have children in the future, change this
	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _walls;
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _walls;
};

