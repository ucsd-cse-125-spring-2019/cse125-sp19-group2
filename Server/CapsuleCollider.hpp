#pragma once

#include "BaseCollider.hpp"

// At the moment this is NOT a capsule collider, but a 2D circle collider. For
// players this seems to get the job done.
class CapsuleCollider : public BaseCollider
{
public:
	CapsuleCollider(BaseState* state) : BaseCollider(state) {};
	~CapsuleCollider() {};

	bool narrowPhase(BaseState* candidate) override
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
				// Distance from sphere to center of AABB
				float distX = std::abs(_state->pos.x - candidate->pos.x);
				float distZ = std::abs(_state->pos.z - candidate->pos.z);

				// Run check on X and Z (sort of an optimization)
				if ((distX > candidate->width / 2 + r1 - COLLISION_THRESHOLD) ||
					(distZ > candidate->depth / 2 + r1 - COLLISION_THRESHOLD))
				{
					return false;
				}

				if ((distX <= candidate->width / 2) ||
					(distZ <= candidate->depth / 2))
				{
					return true;
				}

				// Corners
				if (std::sqrtf((float)(std::pow(distX - candidate->width / 2, 2) +
					std::pow(distZ - candidate->depth / 2, 2))) <= r1 - COLLISION_THRESHOLD)
				{
					return true;
				}

				break;
			}

			} // switch
		}
		return false;
	}

	// Push-back between non-static entities (this) and other entities
	void handleCollision(BaseState* state) override
	{
		BaseState* stateA = _state;
		BaseState* stateB = state;

		// Only perform handling if this, and the object in question is solid
		if (stateA->getSolidity(stateB) && stateB->getSolidity(stateA))
		{
			if (stateB->colliderType == COLLIDER_CAPSULE)
			{
				float rA = (float)std::fmax(stateA->width, stateA->depth) / 2;
				float rB = (float)std::fmax(stateB->width, stateB->depth) / 2;

				// Vector from B to A
				glm::vec3 diff = stateA->pos - stateB->pos;

				float overlap = rA + rB - glm::length(diff);

				// Vector to move circles by
				glm::vec3 correctionVec;

				// Normal case: some displacement between circles
				if (glm::length(diff))
				{
					// How much the circles overlap, and the ratio of overlap to distance
					// between circles
					float ratio = overlap / glm::length(diff);
					correctionVec = diff * ratio;
				}
				// Edge case: objects directly on top each other
				else
				{
					correctionVec = glm::vec3(1, 0, 0) * overlap;
				}

				// If two movable objects, only move halfway
				if (!stateB->isStatic)
				{
					correctionVec /= 2.0f;

					// Apply to B
					stateB->pos -= correctionVec;
				}

				// Apply to A
				stateA->pos += correctionVec;

			} // Capsule <-> Capsule

			else if (stateB->colliderType == COLLIDER_AABB)
			{
				float rA = (float)std::fmax(stateA->width, stateA->depth) / 2;

				// Check which side of box the collision happens on
				float dists[4];
				dists[0] = (stateB->pos.x - stateB->width / 2) - stateA->pos.x; // West
				dists[1] = stateA->pos.x - (stateB->pos.x + stateB->width / 2); // East
				dists[2] = stateA->pos.z - (stateB->pos.z + stateB->depth / 2); // North
				dists[3] = (stateB->pos.z - stateB->depth / 2) - stateA->pos.z; // South

				// Check if collision happens at corner
				glm::vec3 cornerPos = stateB->pos;
				bool inCorner = false;
				if (dists[0] > 0) {
					cornerPos.x -= stateB->width / 2;
					if (dists[2] > 0) {
						cornerPos.z += stateB->depth / 2;
						inCorner = true;
					}
					else if (dists[3] > 0) {
						cornerPos.z -= stateB->depth / 2;
						inCorner = true;
					}
				}
				else if (dists[1] > 0) {
					cornerPos.x += stateB->width / 2;
					if (dists[2] > 0) {
						cornerPos.z += stateB->depth / 2;
						inCorner = true;
					}
					else if (dists[3] > 0) {
						cornerPos.z -= stateB->depth / 2;
						inCorner = true;
					}
				}

				// Collision happens at corner, 
				if (inCorner) {
					// Vector from corner to A
					glm::vec3 diff = stateA->pos - cornerPos;

					// We don't considerate height
					diff.y = 0;

					float overlap = rA - glm::length(diff);

					// Vector to move circles by
					glm::vec3 correctionVec;

					// Normal case: some displacement between circles
					if (glm::length(diff))
					{
						// How much the circles overlap, and the ratio of overlap to distance
						// between circles
						float ratio = overlap / glm::length(diff);
						correctionVec = diff * ratio;
					}
					// Edge case: objects directly on top each other
					else
					{
						correctionVec = glm::vec3(1, 0, 0) * overlap;
					}

					// Apply to A
					stateA->pos += correctionVec;
				}
				else {
					int minIndex = -1;
					float min = FLT_MAX;

					// Get closest edge
					for (int i = 0; i < 4; i++)
					{
						if (dists[i] > 0)
						{
							minIndex = i;
							break;
						}

						if (dists[i] < min)
						{
							min = dists[i];
							minIndex = i;
						}
					}

					glm::vec3 correctionVec = glm::vec3(0);
					switch (minIndex)
					{
					case 0: // West
						correctionVec.x = -(rA - dists[0]);
						break;
					case 1: // East
						correctionVec.x = rA - dists[1];
						break;
					case 2: // North
						correctionVec.z = rA - dists[2];
						break;
					case 3: // South
						correctionVec.z = -(rA - dists[3]);
						break;
					}

					stateA->pos += correctionVec;
				}
			} // Capsule <-> AABB
		} // isSolid
	}
};

