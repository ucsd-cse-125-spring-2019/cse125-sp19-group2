#include "QuadTree.hpp"

QuadTree::~QuadTree()
{
	clear();
}

int QuadTree::getIndex(BaseState * state)
{
	int index = -1;

	bool inTopQuad = ((state->pos.z - state->depth / 2) < _boundary.pos.y) && ((state->pos.z + state->depth / 2) < _boundary.pos.y);
	bool inBottomQuad = ((state->pos.z - state->depth / 2) > _boundary.pos.y);
	bool inLeftQuad = ((state->pos.x - state->width / 2) < _boundary.pos.x) && ((state->pos.x + state->width / 2) < _boundary.pos.x);
	bool inRightQuad = ((state->pos.x - state->width / 2) > _boundary.pos.x);

	if (inLeftQuad)
	{
		if (inTopQuad)
		{
			index = 1;
		}
		else if (inBottomQuad)
		{
			index = 2;
		}
	}
	else if (inRightQuad)
	{
		if (inTopQuad)
		{
			index = 0;
		}
		else if (inBottomQuad)
		{
			index = 3;
		}
	}

	return index;
}

bool QuadTree::insert(BaseState * state)
{
	if (_nw)
	{
		int index = getIndex(state);
		if (index != -1)
		{
			_quads[index]->insert(state);
			return;
		}
	}
	
	_objects.push_back(state);

	if (_objects.size() > QUADTREE_NODE_CAPACITY)
	{
		if (!_nw)
		{
			divide();
		}
		int i = 0;
		while (i < _objects.size())
		{
			int index = getIndex(_objects[i]);
			if (index != -1)
			{
				_quads[index]->insert(_objects[i]);
				_objects.erase(_objects.begin() + index);
			}
			else
			{
				i++;
			}
		}
	}
}

void QuadTree::clear()
{
	_objects.clear();
	if (_nw) delete _nw;
	if (_ne) delete _ne;
	if (_sw) delete _sw;
	if (_se) delete _se;
}

void QuadTree::divide()
{
	BoundingBox nwBoundary = BoundingBox();
	BoundingBox neBoundary = BoundingBox();
	BoundingBox swBoundary = BoundingBox();
	BoundingBox seBoundary = BoundingBox();
	nwBoundary.pos = glm::vec2(_boundary.pos.x - (_boundary.halfWidth / 2), _boundary.pos.y - (_boundary.halfWidth / 2));
	neBoundary.pos = glm::vec2(_boundary.pos.x + (_boundary.halfWidth / 2), _boundary.pos.y - (_boundary.halfWidth / 2));
	swBoundary.pos = glm::vec2(_boundary.pos.x - (_boundary.halfWidth / 2), _boundary.pos.y + (_boundary.halfWidth / 2));
	seBoundary.pos = glm::vec2(_boundary.pos.x + (_boundary.halfWidth / 2), _boundary.pos.y + (_boundary.halfWidth / 2));
	nwBoundary.halfWidth = _boundary.halfWidth / 2;
	neBoundary.halfWidth = _boundary.halfWidth / 2;
	swBoundary.halfWidth = _boundary.halfWidth / 2;
	seBoundary.halfWidth = _boundary.halfWidth / 2;

	_nw = new QuadTree(nwBoundary);
	_ne = new QuadTree(neBoundary);
	_sw = new QuadTree(swBoundary);
	_se = new QuadTree(seBoundary);

	_quads[0] = _ne;
	_quads[1] = _nw;
	_quads[2] = _sw;
	_quads[3] = _se;
}

std::vector<BaseState*> QuadTree::query(std::vector<BaseState*> objectsInRange, BaseState * state)
{
	int index = getIndex(state);
	if (index != -1 && _nw)
	{
		_quads[index]->query(objectsInRange, state);
	}

	objectsInRange.insert(objectsInRange.end(), _objects.begin(), _objects.end());

	return objectsInRange;
}