#include <algorithm>
#include "SBaseEntity.hpp"

std::shared_ptr<BaseState> SBaseEntity::getState()
{
	return _state;
};

std::vector<std::shared_ptr<SBaseEntity>> SBaseEntity::getChildren()
{
	return std::vector<std::shared_ptr<SBaseEntity>>();
}

bool SBaseEntity::isColliding(QuadTree & tree)
{
	return _collider->isColliding(tree);
}

bool SBaseEntity::isColliding(BaseState * state)
{
	return _collider->isColliding(state);
}

std::vector<BaseState*> SBaseEntity::getColliding(QuadTree & tree)
{
	return _collider->getColliding(tree);
}

void SBaseEntity::handleCollision(SBaseEntity * entity)
{
	// Execute lambdas (if any) first
	for (auto f : _collisionHandlers)
	{
		f(this, entity);
	}

	generalHandleCollision(entity);
}

void SBaseEntity::handlePushBack(SBaseEntity * entity)
{
	_collider->handlePushBack(entity->getState().get());
}

void SBaseEntity::initState(bool generateId)
{
	if (generateId)
	{
		_state->id = IdGenerator::getInstance()->getNextId();
	}

	// At origin, looking forward, with 1x1x1 scale
	_state->pos = glm::vec3(0);
	_state->forward = glm::vec3(0, 0, -1);
	_state->up = glm::vec3(0, 1, 0);
	_state->scale = glm::vec3(1);

	// Set default transparency to 1
	_state->transparency = 1.0f;

	// Defaults to static solid object
	_state->isDestroyed = false;
	_state->isStatic = true;
	_state->isSolid = true;

	hasChanged = false;
}

Timer * SBaseEntity::registerTimer(long durationMilliseconds, std::function<void()> f)
{
	auto timer = new Timer(durationMilliseconds, f);
	_timers.push_back(timer);
	return timer;
}

void SBaseEntity::updateTimers()
{
	bool dirty = false;

	// Update all timers and delete them if they're finished
	for (int i = 0; i < _timers.size(); i++)
	{
		if (_timers[i])
		{
			_timers[i]->update();
			if (_timers[i]->isComplete())
			{
				delete _timers[i];
				_timers[i] = nullptr;
				dirty = true;
			}
		}
	}

	// Remove all completed timers from vector
	if (dirty)
	{
		_timers.erase(std::remove(_timers.begin(), _timers.end(), nullptr), _timers.end());
	}
}

void SBaseEntity::rotate(glm::vec3 center, int angle)
{
	if (angle < 1 || angle > 3)
	{
		Logger::getInstance()->warn("Invalid angle: " + std::to_string(angle));
		return;
	}

	glm::vec3 oldRotation = _state->forward;

	float xDiff = _state->pos.x - center.x;
	float zDiff = _state->pos.z - center.z;

	switch (angle)
	{
	case 1:
		_state->pos.x = center.x - zDiff;
		_state->pos.z = center.z + xDiff;
		break;
	case 2:
		_state->pos.x = center.x - xDiff;
		_state->pos.z = center.z - zDiff;
		break;
	case 3:
		_state->pos.x = center.x + zDiff;
		_state->pos.z = center.z - xDiff;
		break;
	}

	glm::vec3 newOrientation = oldRotation;
	for (int i = 0; i < angle; i++)
	{
		newOrientation = rotateOnce(newOrientation);
	}

	_state->forward = newOrientation;

	// Swap width and depth of collider if necessary
	if (angle != 2)
	{
		auto temp = _state->width;
		_state->width = _state->depth;
		_state->depth = temp;

		temp = _state->scale.x;
		_state->scale.x = _state->scale.z;
		_state->scale.z = temp;
	}
}

glm::vec3 SBaseEntity::rotateOnce(glm::vec3 vec)
{
	glm::vec3 newVec(0);
	if (vec.x != 0) {
		newVec.z = -vec.x;
		newVec.x = 0;
	}
	else {
		newVec.x = vec.z;
		newVec.z = 0;
	}
	return newVec;
}
