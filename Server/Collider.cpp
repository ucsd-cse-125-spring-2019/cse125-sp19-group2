#include "Collider.hpp"



Collider::Collider(BaseState * state)
{
	_state = state;
}


Collider::~Collider()
{
}

bool Collider::isColliding(QuadTree & tree)
{
	return narrowPhase(broadPhase(tree));
}

std::vector<BaseState*> Collider::broadPhase(QuadTree & tree)
{
	return tree.query(_state);
}

bool Collider::narrowPhase(std::vector<BaseState*> candidates)
{
	return false; // TODO
}