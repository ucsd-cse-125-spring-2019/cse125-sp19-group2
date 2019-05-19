#pragma once

#include "SBaseEntity.hpp"
#include "AABBCollider.hpp"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#define DOGHOUSE_WALL_WIDTH 0.2f

class SDogHouseEntity : public SBaseEntity
{
public:
	SDogHouseEntity(glm::vec3 pos)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_DOGHOUSE;

		// Ctor parameters
		_state->pos = pos;

		// No collider on the doghouse itself
		_collider = std::make_unique<EmptyCollider>();
		_state->colliderType = COLLIDER_NONE;
		_state->width = 1.2f;
		_state->height = 1.2f;
		_state->depth = 1.2f;

		// Invisible walls to allow dogs to walk into the front of
		// the doghouse
		float backOffset = (_state->depth / 2) - (DOGHOUSE_WALL_WIDTH / 2);
		float sideOffset = (_state->width / 2) - (DOGHOUSE_WALL_WIDTH / 2);
		
		// Wall behind the doghouse
		_backWall = std::make_shared<SBoxEntity>(
			glm::vec3(pos.x, 0, pos.z - backOffset),
			glm::vec3(_state->width, _state->height, DOGHOUSE_WALL_WIDTH));
		_backWall->getState()->transparency = 0.0f;

		// Walls on either side of the doghouse (naming not strictly accurate)
		_leftWall = std::make_shared<SBoxEntity>(
			glm::vec3(pos.x - sideOffset, 0, pos.z + DOGHOUSE_WALL_WIDTH/2),
			glm::vec3(_state->width - DOGHOUSE_WALL_WIDTH, _state->height, DOGHOUSE_WALL_WIDTH));
		_leftWall->getState()->transparency = 0.0f;
		_leftWall->rotate(_leftWall->getState()->pos, 1);

		_rightWall = std::make_shared<SBoxEntity>(
			glm::vec3(pos.x + sideOffset, 0, pos.z + DOGHOUSE_WALL_WIDTH/2),
			glm::vec3(_state->width - DOGHOUSE_WALL_WIDTH, _state->height, DOGHOUSE_WALL_WIDTH));
		_rightWall->getState()->transparency = 0.0f;
		_rightWall->rotate(_rightWall->getState()->pos, 1);

		// Sensor for dogs. Performs actual logic on the doghouse
		auto sensorBox = std::make_shared<SBoxEntity>(
			glm::vec3(pos.x, 0, pos.z - _state->depth / 4),
			glm::vec3(_state->width - DOGHOUSE_WALL_WIDTH * 2, 0.5, _state->depth / 3));
		sensorBox->getState()->transparency = 0.0f;
		sensorBox->getState()->isSolid = false;
		
		// Register actual handling
		sensorBox->onCollision([&](SBaseEntity* entity, SBaseEntity* collidingEntity)
		{
			if (collidingEntity->getState()->type == ENTITY_DOG)
			{
				Logger::getInstance()->debug("Dog is \"teleporting\"");
			}
		});

		_children.push_back(_backWall);
		_children.push_back(_leftWall);
		_children.push_back(_rightWall);
		_children.push_back(sensorBox);
	};
	~SDogHouseEntity() {};

	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _children;
	}

	// Ensure children are also rotated
	void rotate(glm::vec3 center, int angle) override
	{
		SBaseEntity::rotate(center, angle);

		for (auto& child : _children)
		{
			child->rotate(center, angle);
		}
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _children;

	std::shared_ptr<SBaseEntity> _backWall;
	std::shared_ptr<SBaseEntity> _leftWall;
	std::shared_ptr<SBaseEntity> _rightWall;
};

