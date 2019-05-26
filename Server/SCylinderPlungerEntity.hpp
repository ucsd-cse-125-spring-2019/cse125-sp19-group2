#pragma once

#include "SCylinderEntity.hpp"

class SCylinderPlungerEntity : public SCylinderEntity
{
public:
	SCylinderPlungerEntity(
		glm::vec3 pos,
		glm::vec3 scale) : SCylinderEntity(pos, scale)
	{
		_state->type = ENTITY_HIT_PLUNGER;

		_state->setSolidity([&](BaseState* entity, BaseState* collidingEntity)
		{
			return (collidingEntity->type == ENTITY_PLUNGER);
		});
	};
	~SCylinderPlungerEntity() {};
};

