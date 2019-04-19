#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "BaseState.hpp"

#define QUADTREE_NODE_CAPACITY 4

// "borrowed" from https://gamedevelopment.tutsplus.com/tutorials/quick-tip-use-quadtrees-to-detect-likely-collisions-in-2d-space--gamedev-374

// Internal representation of a square, axis-aligned bounding box
struct BoundingBox
{
	glm::vec2 pos;		// Center of box
	double halfWidth;	// Half of width of box

	bool containsPoint(glm::vec2 point)
	{
		return (
			point.x >= (pos.x - halfWidth) &&
			point.x <= (pos.x + halfWidth) &&
			point.y >= (pos.y - halfWidth) &&
			point.y <= (pos.y + halfWidth));
	}

	bool intersects(BoundingBox * bb)
	{
		return (
			(bb->pos.x + bb->halfWidth) >= (pos.x - halfWidth) &&
			(bb->pos.x - bb->halfWidth) <= (pos.x + halfWidth) &&
			(bb->pos.y + bb->halfWidth) >= (pos.y - halfWidth) &&
			(bb->pos.y - bb->halfWidth) <= (pos.y + halfWidth));
	}

	bool intersects(BaseState * state)
	{
		return (
			(state->pos.x + state->width / 2) >= (pos.x - halfWidth) &&
			(state->pos.x - state->width / 2) <= (pos.x + halfWidth) &&
			(state->pos.z + state->depth / 2) >= (pos.y - halfWidth) &&
			(state->pos.z - state->depth / 2) <= (pos.y + halfWidth));
	}
};

class QuadTree
{
public:
	QuadTree(BoundingBox boundary)
	{
		this->_boundary = boundary;
		_quads = (QuadTree**)malloc(sizeof(QuadTree*) * 4);
	}

	~QuadTree();

	void insert(BaseState * state);
	void clear();
	void divide();
	int getIndex(BaseState * state);
	bool intersects(BaseState * state);
	std::vector<BaseState*> query(std::vector<BaseState*> objectsInRange, BaseState * state);

private:
	// Square bounding box defining this node's area
	BoundingBox _boundary;

	// Objects in this node
	std::vector<BaseState*> _objects;

	// Subtrees
	QuadTree * _nw;
	QuadTree * _ne;
	QuadTree * _sw;
	QuadTree * _se;
	QuadTree ** _quads;
};
