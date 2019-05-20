#pragma once
#include "SBoxEntity.hpp"

class STriggerEntity : public SBoxEntity
{
public:
	STriggerEntity(
		glm::vec3 pos,
		glm::vec3 scale,
		glm::vec3 forward) : SBoxEntity(pos, scale)
	{
		_state->type = ENTITY_TRIGGER;
		_state->forward = forward;
	};
	~STriggerEntity() {};
};