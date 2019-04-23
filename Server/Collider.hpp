#pragma once
#include <memory>

#include "Shared/QuadTree.hpp"
#include "Shared/BaseState.hpp"

class Collider
{
private:
	BaseState * _state;
	
	std::vector<BaseState*> broadPhase(QuadTree & tree);
	bool narrowPhase(std::vector<BaseState*> candidates);

public:
	Collider(BaseState * state);
	~Collider();

	bool isColliding(QuadTree & tree);
};

