#pragma once
#include <memory>

#include "Shared/QuadTree.hpp"
#include "Shared/BaseState.hpp"

class BaseCollider
{
public:
	BaseCollider(BaseState* state)
	{
		_state = state;
	}

	~BaseCollider() {};

	bool isColliding(QuadTree & tree)
	{
		return narrowPhase(broadPhase(tree));
	};

protected:
	BaseState * _state;

	std::vector<BaseState*> broadPhase(QuadTree & tree)
	{
		return tree.query(_state);
	}

	virtual bool narrowPhase(std::vector<BaseState*> candidates) = 0;
};

