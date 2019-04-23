#pragma once

#include <memory>

#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/QuadTree.hpp"
#include "Collider.hpp"

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
	// Update function, called every tick
	virtual void update(
			QuadTree & tree,
			std::vector<std::shared_ptr<GameEvent>> events) = 0;

    // All server objects must have a state to send to the client.
	// By default, this function should return the state object only if it was
	// changed. Otherwise it returns null. If ignoreUpdateStatus is set to true,
	// this should return the state object regardless.
    virtual std::shared_ptr<BaseState> getState(bool ignoreUpdateStatus = false) = 0;
	virtual bool isColliding(QuadTree & tree) = 0;

	// TODO: add more server-specific functions that are object-agnostic

protected:
	// TODO: server-specific state goes here
	bool _isStatic;		// Whether the object's state can be changed
	bool _hasChanged;	// If object state has changed during the last iteration
	Collider * _collider; // bounding box state info
};

