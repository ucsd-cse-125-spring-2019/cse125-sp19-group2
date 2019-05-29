#pragma once

#include "SBaseEntity.hpp"
#include "SCylinderEntity.hpp"
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
		// Create cylinder with slightly larger radius
		auto dogSensor = std::make_shared<SCylinderEntity>(
			pos,
			glm::vec3(_state->width + 1.5f, 0.1f, _state->depth + 1.5f));
		dogSensor->getState()->isSolid = false;

		// Collision sensor for dogs
		dogSensor->onCollision([&](SBaseEntity * entity, SBaseEntity * collidingEntity)
			{
				if (collidingEntity->getState()->type == ENTITY_DOG)
				{
					SDogEntity* collidingDog = static_cast<SDogEntity*>(collidingEntity);
					collidingDog->setNearFountain(true);

					DogState* dogState = static_cast<DogState*>(collidingEntity->getState().get());
					dogState->message = "Drink (Left click / A)";
					collidingEntity->hasChanged = true;

					// Get unit vector of dog to fountain
					glm::vec3 fountainDir = glm::normalize(_state->pos - collidingDog->getState()->pos);
					collidingDog->targetDir = fountainDir;

					// Send position to interpolate to (edge of fountain)
					collidingDog->targetPos = _state->pos + ((-fountainDir) * (_state->width/2 + collidingDog->getState()->width/2 + 0.01f));
				}
			});

		_children.push_back(dogSensor);
	};
	~SFountainEntity() {};

	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _children;
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _children;
};
