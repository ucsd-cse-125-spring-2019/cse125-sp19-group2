#pragma once

#include <memory>

#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/QuadTree.hpp"
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
    virtual std::shared_ptr<BaseState> getState() = 0;

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
		 
	// TODO: add more server-specific functions that are object-agnostic

protected:
	// TODO: server-specific state goes here
	std::unique_ptr<BaseCollider> _collider; // bounding box state info
};

