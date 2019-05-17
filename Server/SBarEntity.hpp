#pragma once
#include "SBoxEntity.hpp"

class SBarEntity : public SBoxEntity
{
public:
	SBarEntity(
		glm::vec3 pos,
		glm::vec3 scale) : SBoxEntity(pos, scale)
	{
		_state->type = ENTITY_BAR;
	};
	~SBarEntity() {};
};