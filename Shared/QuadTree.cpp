#include "QuadTree.hpp"

QuadTree::~QuadTree()
{
	clear();
}

int QuadTree::getIndex(BaseState * state)
{
	int index = -1;
	float objBottom = (state->pos.z - state->depth / 2);
	float objTop = (state->pos.z + state->depth / 2);
	float objLeft = (state->pos.x - state->width / 2);
	float objRight = (state->pos.x + state->width / 2);
	float quadBottom = _boundary.pos.y - _boundary.halfWidth;
	float quadHorizMid = _boundary.pos.y;
	float quadTop = _boundary.pos.y + _boundary.halfWidth;
	float quadLeft = _boundary.pos.x - _boundary.halfWidth;
	float quadVertMid = _boundary.pos.x;
	float quadRight = _boundary.pos.x + _boundary.halfWidth;

	bool inBottomQuad = (objBottom > quadBottom) && (objTop < quadHorizMid);
	bool inTopQuad = (objBottom > quadHorizMid) && (objTop < quadTop);
	bool inLeftQuad = (objLeft > quadLeft) && (objRight < quadVertMid);
	bool inRightQuad = (objLeft > quadVertMid) && (objRight < quadRight);

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

void QuadTree::insert(BaseState * state)
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
				_objects.erase(_objects.begin() + i);
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
	nwBoundary.pos = glm::vec2(_boundary.pos.x - (_boundary.halfWidth / 2), _boundary.pos.y + (_boundary.halfWidth / 2));
	neBoundary.pos = glm::vec2(_boundary.pos.x + (_boundary.halfWidth / 2), _boundary.pos.y + (_boundary.halfWidth / 2));
	swBoundary.pos = glm::vec2(_boundary.pos.x - (_boundary.halfWidth / 2), _boundary.pos.y - (_boundary.halfWidth / 2));
	seBoundary.pos = glm::vec2(_boundary.pos.x + (_boundary.halfWidth / 2), _boundary.pos.y - (_boundary.halfWidth / 2));
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
		objectsInRange = _quads[index]->query(objectsInRange, state);
	}
	else if (index == -1 && _ne)
	{
		
	}

	objectsInRange.insert(objectsInRange.end(), _objects.begin(), _objects.end());

	return objectsInRange;
}

bool QuadTree::intersects(BaseState * state)
{
	return _boundary.intersects(state);
}