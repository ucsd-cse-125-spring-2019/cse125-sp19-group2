#pragma once

#include "SBoxEntity.hpp"

class SFenceEntity : public SBoxEntity
{
public:
	SFenceEntity(
		glm::vec3 pos,
		glm::vec3 scale) : SBoxEntity(pos, scale)
	{
		_state->type = ENTITY_FENCE;
	};
	~SFenceEntity() {};
};

