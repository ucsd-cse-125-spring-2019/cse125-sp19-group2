#pragma once
#include "SBoxEntity.hpp"

class SGateEntity : public SBoxEntity
{
public:
	SGateEntity(
		glm::vec3 pos,
		glm::vec3 scale) : SBoxEntity(pos, scale)
	{
		_state->type = ENTITY_GATE;
		_state->colliderType = COLLIDER_GATE;
	};
	~SGateEntity() {};
};