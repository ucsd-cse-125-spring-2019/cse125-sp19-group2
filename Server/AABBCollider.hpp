#pragma once

#include "BaseCollider.hpp"

class AABBCollider : public BaseCollider
{
public:
	AABBCollider(BaseState* state) : BaseCollider(state) {};
	~AABBCollider() {};

	bool narrowPhase(BaseState* candidate)
	{
		// Only check the candidate if it's not itself
		if (candidate->id != _state->id)
		{
			switch (candidate->colliderType)
			{

			// Case 1: candidate is also a box collider
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

