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

		// Basic AABB. This will probably have to change to at least
		// three child AABB colliders, so the dog can actually go
		// inside the doghouse.
		//_collider = std::make_unique<AABBCollider>(_state.get());
		//_state->colliderType = COLLIDER_AABB;
		_state->width = 1.2f;
		_state->height = 1.2f;
		_state->depth = 1.2f;
		_collider = std::make_unique<EmptyCollider>();
		_state->colliderType = COLLIDER_NONE;

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
		_leftWall->rotate(glm::vec3(1.0f, 0, 0));

		_rightWall = std::make_shared<SBoxEntity>(
			glm::vec3(pos.x + sideOffset, 0, pos.z + DOGHOUSE_WALL_WIDTH/2),
			glm::vec3(_state->width - DOGHOUSE_WALL_WIDTH, _state->height, DOGHOUSE_WALL_WIDTH));
		_rightWall->getState()->transparency = 0.0f;
		_rightWall->rotate(glm::vec3(1.0f, 0, 0));

		_children.push_back(_backWall);
		_children.push_back(_leftWall);
		_children.push_back(_rightWall);
	};
	~SDogHouseEntity() {};

	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _children;
	}

	// Ensure children are also rotated
	void rotate(glm::vec3 orientation) override
	{
		// Get angle of rotation first
		float angle = glm::angle(_state->forward, orientation);

		// Rotate this object
		SBaseEntity::rotate(orientation);

		// Then the children
		_backWall->rotate(orientation);
		glm::vec2 backDiff = _backWall->getState()->pos - _state->pos;
		glm::vec2 rotatedBack = glm::rotate(backDiff, angle);
		rotatedBack /= glm::length(rotatedBack);
		rotatedBack *= glm::length(backDiff);
		_backWall->getState()->pos = _state->pos + glm::vec3(rotatedBack.x, 0, rotatedBack.y);
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _children;

	std::shared_ptr<SBaseEntity> _backWall;
	std::shared_ptr<SBaseEntity> _leftWall;
	std::shared_ptr<SBaseEntity> _rightWall;
};

