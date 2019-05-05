#pragma once
#include <memory>

#include "Shared/QuadTree.hpp"
#include "Shared/BaseState.hpp"

#define COLLISION_THRESHOLD 0.001

class BaseCollider
{
public:
	BaseCollider()
	{
		_state = nullptr;
	}

	BaseCollider(BaseState* state)
	{
		_state = state;
	}

	~BaseCollider() {};

	// Checks for ANY collision with objects inside a quadtree
	bool isColliding(QuadTree & tree)
	{
		// TODO: cache results
		for (auto& candidate : broadPhase(tree))
		{
			if (narrowPhase(candidate))
			{
				return true;
			}
		}
		return false;
	};

	// Checks for collisions against a specific object
	bool isColliding(BaseState* state)
	{
		return narrowPhase(state);
	};

	// Gets a vector of all colliding objects
	std::vector<BaseState*> getColliding(QuadTree & tree)
	{
		auto resultVec = std::vector<BaseState*>();

		// TODO: cache results
		for (auto& candidate : broadPhase(tree))
		{
			if (narrowPhase(candidate))
			{
				resultVec.push_back(candidate);
			}
		}

		return resultVec;
	};

	// Handle push-back between entities. By default, does nothing
	virtual void handleCollision(BaseState* state) {};

protected:
	BaseState* _state;

	// Broad-phase collision detection always uses a QuadTree
	virtual std::vector<BaseState*> broadPhase(QuadTree & tree)
	{
		return tree.query(_state);
	};

	// Only function that colliders need to implement
	virtual bool narrowPhase(BaseState* candidate) = 0;
};

