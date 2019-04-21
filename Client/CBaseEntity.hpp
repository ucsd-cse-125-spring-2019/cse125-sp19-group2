#pragma once

#include <memory>

#include "Shared/BaseState.hpp"
#include "Camera.hpp"

/*
** This is an interface that any graphics objects on the client side must be
** derived from. Do not instantiate objects of this class.
**
** E.g.
** class CDogBone : public CBaseEntity {..}
** class CPlayer : public CBaseEntity {..}
** etc.
*/
class CBaseEntity
{
private:
	// TODO: models, shaders, textures, matrices, etc. Don't forget getters
	// and setters so children have access (or switch to protected).
public:
	// Name this whatever you like; it should be called every frame
	virtual void render(std::unique_ptr<Camera> const & camera) = 0;

    // Every child must implement this
    virtual void updateState(std::shared_ptr<BaseState> state) = 0;

    // This is optional, but might make our lives easier. Remove if you feel
    // that it is not necessary
    virtual uint32_t getId() = 0;

	// TODO: add any functionality here that is needed in every client-side
	// object.
};

