#pragma once

#include "SBoxEntity.hpp"
#include "SBarEntity.hpp"
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

		float xScale = scale.x / 3;
		float zScale = scale.z / 3;

		// North left wall
		_walls.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x - xScale, pos.y, pos.z + scale.z / 2 - JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		));

		// North right wall
		_walls.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x + xScale, pos.y, pos.z + scale.z / 2 - JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		));

		// South left wall
		_walls.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x - xScale, pos.y, pos.z - scale.z / 2 + JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		));

		// South right wall
		_walls.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x + xScale, pos.y, pos.z - scale.z / 2 + JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		));

		// West up wall
		_walls.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x - scale.x / 2 + JAIL_WALL_WIDTH / 2, pos.y, pos.z - zScale + JAIL_WALL_WIDTH / 2),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - JAIL_WALL_WIDTH)
		));

		// West down wall
		_walls.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x - scale.x / 2 + JAIL_WALL_WIDTH / 2, pos.y, pos.z + zScale - JAIL_WALL_WIDTH / 2),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - JAIL_WALL_WIDTH)
			));

		// East up wall
		_walls.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x + scale.x / 2 - JAIL_WALL_WIDTH / 2, pos.y, pos.z - zScale + JAIL_WALL_WIDTH / 2),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - JAIL_WALL_WIDTH)
			));

		// East down wall
		_walls.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x + scale.x / 2 - JAIL_WALL_WIDTH / 2, pos.y, pos.z + zScale - JAIL_WALL_WIDTH / 2),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - JAIL_WALL_WIDTH)
		));

		// TODO: change below to gateEntity after completed
		//// North gate
		//_walls.push_back(std::make_shared<SBarEntity>(
		//	glm::vec3(pos.x, pos.y, pos.z + scale.z / 2 - JAIL_WALL_WIDTH / 2),
		//	glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		//	));

		//// South gate
		//_walls.push_back(std::make_shared<SBarEntity>(
		//	glm::vec3(pos.x, pos.y, pos.z - scale.z / 2 + JAIL_WALL_WIDTH / 2),
		//	glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		//	));

		//// West gate
		//_walls.push_back(std::make_shared<SBarEntity>(
		//	glm::vec3(pos.x - scale.x / 2 + JAIL_WALL_WIDTH / 2, pos.y, pos.z),
		//	glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale)
		//));

		//// East gate
		//_walls.push_back(std::make_shared<SBarEntity>(
		//	glm::vec3(pos.x + scale.x / 2 - JAIL_WALL_WIDTH / 2, pos.y, pos.z),
		//	glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale)
		//));
	};
	~SJailEntity() {};

	// If walls have children in the future, change this
	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _walls;
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _walls;
};

