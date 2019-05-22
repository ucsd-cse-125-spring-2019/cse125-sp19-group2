#pragma once

#include "SBaseEntity.hpp"
#include "AABBCollider.hpp"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#define DOGHOUSE_COOLDOWN_SECS 5
#define DOGHOUSE_WALL_WIDTH 0.2f

class SDogHouseEntity : public SBaseEntity
{
public:
	SDogHouseEntity(
		glm::vec3 pos,
		std::vector<std::shared_ptr<SBaseEntity>>* dogHouses)
	{
		_state = std::make_shared<BaseState>();

		_dogHouses = dogHouses;

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
		auto backWall = std::make_shared<SBoxEntity>(
			glm::vec3(pos.x, 0, pos.z - backOffset),
			glm::vec3(_state->width, _state->height, DOGHOUSE_WALL_WIDTH));
		backWall->getState()->transparency = 0.0f;
		_walls.push_back(backWall);

		// Walls on either side of the doghouse (naming not strictly accurate)
		auto leftWall = std::make_shared<SBoxEntity>(
			glm::vec3(pos.x - sideOffset, 0, pos.z + DOGHOUSE_WALL_WIDTH/2),
			glm::vec3(_state->width - DOGHOUSE_WALL_WIDTH, _state->height, DOGHOUSE_WALL_WIDTH));
		leftWall->getState()->transparency = 0.0f;
		leftWall->rotate(leftWall->getState()->pos, 1);
		_walls.push_back(leftWall);

		auto rightWall = std::make_shared<SBoxEntity>(
			glm::vec3(pos.x + sideOffset, 0, pos.z + DOGHOUSE_WALL_WIDTH/2),
			glm::vec3(_state->width - DOGHOUSE_WALL_WIDTH, _state->height, DOGHOUSE_WALL_WIDTH));
		rightWall->getState()->transparency = 0.0f;
		rightWall->rotate(rightWall->getState()->pos, 1);
		_walls.push_back(rightWall);

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
				// Choose another dog house to teleport to
				for (int i = 0; i < _dogHouses->size(); i++)
				{
					if ((*_dogHouses)[i]->getState()->id != _state->id)
					{
						auto house = (*_dogHouses)[i];
						std::shared_ptr<SDogHouseEntity> castHouse = std::static_pointer_cast<SDogHouseEntity>(house);

						// Check cooldown first
						auto result = _cooldowns.find(collidingEntity->getState()->id);
						if (result != _cooldowns.end())
						{
							// Check cooldown, return if too soon
							auto elapsed = std::chrono::steady_clock::now() - result->second;
							if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() < DOGHOUSE_COOLDOWN_SECS)
							{
								return;
							}
							else
							{
								_cooldowns.erase(result);

								// Find and erase from other house
								castHouse->_cooldowns.erase(collidingEntity->getState()->id);
							}
						}

						// Teleport to this house
						collidingEntity->getState()->pos = house->getState()->pos;

						// Reset cooldowns
						_cooldowns.insert({ collidingEntity->getState()->id, std::chrono::steady_clock::now() });
						castHouse->_cooldowns.insert({ collidingEntity->getState()->id, std::chrono::steady_clock::now() });

						// Set source house as non-solid for next tick
						SPlayerEntity* dogEntity = static_cast<SPlayerEntity*>(collidingEntity);
						for (auto& wall : _walls)
						{
							wall->getState()->isSolid = false;
						}
						break;
					}
				}
			}
		});

		_children.push_back(backWall);
		_children.push_back(leftWall);
		_children.push_back(rightWall);
		_children.push_back(sensorBox);
	};
	~SDogHouseEntity() {};

	// Reset solidity of doghouse walls
	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		for (auto& wall : _walls)
		{
			wall->getState()->isSolid = true;
		}
	}

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

	// Map of dog ID's to cooldowns
	std::unordered_map<uint32_t, std::chrono::time_point<std::chrono::steady_clock>> _cooldowns;

private:
	std::vector<std::shared_ptr<SBaseEntity>>* _dogHouses;
	std::vector<std::shared_ptr<SBaseEntity>> _children;
	std::vector<std::shared_ptr<SBaseEntity>> _walls;
};

