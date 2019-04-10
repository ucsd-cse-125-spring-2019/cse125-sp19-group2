#pragma once

#include "Shared/GameEntity.hpp"

/*
** This is an interface that any graphics objects on the client side must be
** derived from, along with GameEntity. Do not instantiate objects of this
** class. Only objects derived from both GameEntity (or its descendant) and
** this class should be instantiated client-side. Here are a few examples of
** real client-side objects that might be instantiated:
**
** class CDogBone : public CGameEntity, public DogBone {..}
** class CPlayer : public CGameEntity, public Player {..}
** class CLight : public CGameEntity, public Light {..}
*/
class CGameEntity
{
private:
	// TODO: models, shaders, textures, matrices, etc. Don't forget getters
	// and setters so children have access (or switch to protected).
public:
	// Name this whatever you like; it should be called every frame
	virtual void render() = 0;

	// TODO: add any functionality here that is needed in every client-side
	// object.
};

