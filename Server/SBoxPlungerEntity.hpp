#pragma once

#include "SBoxEntity.hpp"

class SBoxPlungerEntity : public SBoxEntity
{
public:
	SBoxPlungerEntity(
		glm::vec3 pos,
		glm::vec3 scale) : SBoxEntity(pos, scale)
	{
		_state->type = ENTITY_HIT_PLUNGER;
	};
	~SBoxPlungerEntity() {};
};

