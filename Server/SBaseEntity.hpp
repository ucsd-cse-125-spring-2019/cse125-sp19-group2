#pragma once

#include <memory>

#include "Shared/BaseState.hpp"

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
private:
	// TODO: server-specific state goes here

public:
	// Update function, called every tick
	virtual void update() = 0;

    // All server objects must have a state to send to the client
    virtual std::shared_ptr<BaseState> getState() = 0;

	// TODO: add more server-specific functions that are object-agnostic
};

