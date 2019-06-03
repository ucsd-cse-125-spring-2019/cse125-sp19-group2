#pragma once

#include "SCylinderEntity.hpp"
#include <algorithm>

class SNetEntity : public SCylinderEntity
{
public:
	SNetEntity(
		glm::vec3 pos,
		float velocity,
		float maxDistance) : SCylinderEntity(pos, glm::vec3(0.75f, 1.0f, 0.75f))
	{
		_state->type = ENTITY_NET;
		_state->isSolid = false;
		_velocity = velocity;
		_maxDistance = maxDistance;
	};

	void updateDistance(glm::vec3 pos, glm::vec3 forward)
	{
		_curDistance += _velocity;
		if (_curDistance > _maxDistance)
			_curDistance = _maxDistance;
		_state->pos = pos + forward * _curDistance;
		hasChanged = true;
	}

	~SNetEntity() {};

private:
	float _curDistance = 0;
	float _velocity = 0;
	float _maxDistance = 0;
};

