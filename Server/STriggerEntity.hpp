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
		origForward = forward;

	};

	void updateForward(int degree) {
		curDegree += degree;
		curDegree %= 360;
		_state->forward = origForward * (float)curDegree + origForward;
		hasChanged = true;
	}

	//virtual void update(std::vector<std::shared_ptr<GameEvent>> events) override
	//{
	//	hasChanged = false;
	//}

	~STriggerEntity() {};

private:
	glm::vec3 origForward;
	int curDegree;
};