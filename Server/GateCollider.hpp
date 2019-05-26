#pragma once
#pragma once

#include "BaseCollider.hpp"

class GateCollider : public BaseCollider
{
public:
	GateCollider(BaseState* state) : BaseCollider(state) {};
	~GateCollider() {};

	bool narrowPhase(BaseState* candidate)
	{
		// Only check the candidate if it's not itself
		if (candidate->id != _state->id)
		{
			switch (candidate->colliderType)
			{

				// Case 1: candidate is also a box collider
			case COLLIDER_GATE:
			case COLLIDER_AABB:
			{
				// TODO
				break;
			}

			// Case 2: candidate is a capsule collider
			case COLLIDER_CAPSULE:
			{
				// TODO
				break;
			}
			} // switch
		}
		return false;
	}
};

