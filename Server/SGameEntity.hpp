#pragma once

#include "Shared/GameEntity.hpp"

/*
** As with CGameEntity, this is an abstract class, and cannot be instantiated.
** Use only as a base class for server objects.
** 
** E.g.
** class SPlayer : public SGameEntity, public Player {..}
** class SDogBone : public SGameEntity, public DogBone {..} 
*/
class SGameEntity
{
private:
	// TODO: server-specific state goes here
public:
	// Update function, called every tick
	virtual void update() = 0;

	// TODO: add more server-specific functions that are object-agnostic
};

