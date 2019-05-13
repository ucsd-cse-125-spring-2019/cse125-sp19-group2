#pragma once

#include <memory>

#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/QuadTree.hpp"
#include "IdGenerator.hpp"
#include "BaseCollider.hpp"

/*
** As with CBaseEntity, this is an abstract class, and cannot be instantiated.
** Use only as a base class for server objects.
** 
** E.g.
** class SPlayer : public SBaseEntity {..}
** class SDogBone : public SBaseEntity {..} 
** etc.
*/
class SBaseEntity
{
public:
	bool hasChanged;	// If object state has changed during the last iteration

	// Update function, called every tick
	virtual void update(std::vector<std::shared_ptr<GameEvent>> events) = 0;

    // All server objects must have a state to send to the client.
	virtual std::shared_ptr<BaseState> getState()
	{
		return _state;
	};

	// For objects with "scene graphs". By default, no kids
	virtual	std::vector<std::shared_ptr<SBaseEntity>> getChildren()
	{
		return std::vector<std::shared_ptr<SBaseEntity>>();
	};

	// Wrappers for colliders
	virtual bool isColliding(QuadTree & tree)
	{
		return _collider->isColliding(tree);
	};

	virtual bool isColliding(BaseState* state)
	{
		return _collider->isColliding(state);
	};

	virtual std::vector<BaseState*> getColliding(QuadTree & tree)
	{
		return _collider->getColliding(tree);
	};

	// Called by the CollisionManager; handle collision with specific object
	virtual void handleCollision(std::shared_ptr<SBaseEntity> entity)
	{
		// Basic "bumping away" logic
		_collider->handleCollision(entity->getState().get());
	};

	// Initializes state struct to some sane defaults
	virtual void initState(bool generateId = true)
	{
		if (generateId)
		{
			_state->id = IdGenerator::getInstance()->getNextId();
		}

		// At origin, looking forward, with 1x1x1 scale
		_state->pos = glm::vec3(0);
		_state->forward = glm::vec3(0, 0, 1);
		_state->up = glm::vec3(0, 1, 0);
		_state->scale = glm::vec3(1);

		// Defaults to static solid object
		_state->isDestroyed = false;
		_state->isStatic = true;
		_state->isSolid = true;

		hasChanged = false;
	};
		 
	// TODO: add more server-specific functions that are object-agnostic

protected:
	// TODO: server-specific state goes here
	std::unique_ptr<BaseCollider> _collider; // bounding box state info
	std::shared_ptr<BaseState> _state;
};

