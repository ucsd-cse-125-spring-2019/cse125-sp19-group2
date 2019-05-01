#pragma once

#include <memory>
#include "IdGenerator.hpp"
#include "SBaseEntity.hpp"
#include "Shared/BaseState.hpp"

class SBoxEntity : public SBaseEntity
{
public:
	SBoxEntity(
		glm::vec3 pos,
		glm::vec3 forward,
		glm::vec3 scale)
	{
		// Allocate a state struct and initialize. Modify as necessary for more
		// sane defaults
		_state = std::make_shared<BaseState>();

		// ID and type
		_state->id = IdGenerator::getInstance()->getNextId();
		_state->type = ENTITY_BOX;

		// At origin by default
		_state->pos = pos;

		// Looking forward (along Z axis)
		_state->forward = glm::vec3(0, 0, 1);
		_state->up = glm::vec3(0, 1, 0);

		// Base scale on constructor args
		_state->scale = scale;

		// Collider stuff
		_state->width = scale.x;
		_state->height = scale.y;
		_state->depth = scale.z;
		_state->colliderType = COLLIDER_AABB;

		_state->isDestroyed = false;
		_state->isStatic = true;

		hasChanged = false;
	};
	~SBoxEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		// Update does nothing for box entities
	}

	std::shared_ptr<BaseState> getState() override
	{
		return _state;
	}

private:
	std::shared_ptr<BaseState> _state;
};

