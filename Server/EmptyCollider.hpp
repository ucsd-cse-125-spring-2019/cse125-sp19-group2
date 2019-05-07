#pragma once

#include "BaseCollider.hpp"

// Collider that will not collide with anything
class EmptyCollider : public BaseCollider
{
public:
	EmptyCollider() {};

	// Override broad phase and narrow phase to return nothing
	std::vector<BaseState*> broadPhase(QuadTree& tree) override
	{
		return std::vector<BaseState*>();
	}

	bool narrowPhase(BaseState* candidate) override
	{
		return false;
	}
};

