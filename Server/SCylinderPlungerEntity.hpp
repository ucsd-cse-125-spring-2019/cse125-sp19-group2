#pragma once

#include "SCylinderEntity.hpp"

class SBoxPlungerEntity : public SCylinderEntity
{
public:
	SBoxPlungerEntity(
		glm::vec3 pos,
		glm::vec3 scale) : SCylinderEntity(pos, scale)
	{
		_state->type = ENTITY_HIT_PLUNGER;
	};
	~SBoxPlungerEntity() {};
};

