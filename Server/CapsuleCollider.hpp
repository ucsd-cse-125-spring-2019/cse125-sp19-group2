#pragma once

#include "BaseCollider.hpp"

// At the moment this is NOT a capsule collider, but a 2D circle collider. For
// players this seems to get the job done.
class CapsuleCollider : public BaseCollider
{
public:
	CapsuleCollider(BaseState* state) : BaseCollider(state) {};
	~CapsuleCollider() {};

	bool narrowPhase(BaseState* candidate)
	{
		// using 2D sphere-sphere collision for now since there is no rotation or height changing to players
		double r1 = std::fmax(_state->width, _state->depth) / 2;

		// Only check the candidate if it's not itself
		if (candidate->id != _state->id)
		{
			switch (candidate->colliderType)
			{

			// Case 1: candidate is also a capsule collider
			case COLLIDER_CAPSULE:
			{
				double r2 = std::fmax(candidate->width, candidate->depth) / 2;
				double dist = std::sqrt(std::pow(_state->pos.x - candidate->pos.x, 2) + std::pow(_state->pos.z - candidate->pos.z, 2));
				if (dist <= r1 + r2 - COLLISION_THRESHOLD)
				{
					return true;
				}
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
		return false;
	}
};

