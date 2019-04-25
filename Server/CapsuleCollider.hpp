#pragma once

#include "BaseCollider.hpp"

class CapsuleCollider : public BaseCollider
{
public:
	CapsuleCollider(BaseState* state) : BaseCollider(state) {};
	~CapsuleCollider() {};

	bool narrowPhase(std::vector<BaseState*> candidates)
	{
		// using 2D sphere-sphere collision for now since there is no rotation or height changing to players
		double r1 = std::fmax(_state->width, _state->depth) / 2;

		for (auto curCandidate : candidates)
		{
			switch (curCandidate->colliderType)
			{
			// Case 1: candidate is also a capsule collider
			case COLLIDER_CAPSULE:
				// Only check the candidate if it's not itself
				if (curCandidate->id != _state->id)
				{
					double r2 = std::fmax(curCandidate->width, curCandidate->depth) / 2;
					double dist = std::sqrt(std::pow(_state->pos.x - curCandidate->pos.x, 2) + std::pow(_state->pos.z - curCandidate->pos.z, 2));
					if (dist <= r1 + r2)
					{
						return true;
					}
				}
				break;
			
			// Case 2: candidate is a box collider
			case COLLIDER_AABB:
				// TODO
				break;
			}
		}
		return false;
	}
};

