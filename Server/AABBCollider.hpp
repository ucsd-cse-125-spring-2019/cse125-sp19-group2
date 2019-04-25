#pragma once

#include "BaseCollider.hpp"

class AABBCollider : public BaseCollider
{
public:
	AABBCollider(BaseState* state) : BaseCollider(state) {};
	~AABBCollider() {};

	bool narrowPhase(std::vector<BaseState*> candidates)
	{
		// Run narrow phase check on every possible candidate
		for (auto curCandidate : candidates)
		{
			// Only check the candidate if it's not itself
			if (curCandidate->id != _state->id)
			{
				switch (curCandidate->colliderType)
				{

				// Case 1: candidate is also a capsule collider
				case COLLIDER_CAPSULE:
				{
					// TODO
					break;
				}

				// Case 2: candidate is a box collider
				case COLLIDER_AABB:
				{
					// TODO
					break;
				}

				} // switch
			}
		}
		return false;
	}
};

