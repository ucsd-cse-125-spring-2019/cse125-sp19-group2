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
	virtual void update(std::vector<std::shared_ptr<GameEvent>> events) {};

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

	// Registers custom collision handler for this object. Called inside
	// handleCollision()
	template <typename T>
	void onCollision(T&& lambda)
	{
		_collisionHandlers.push_back(lambda);
	}

	// Called by the CollisionManager; handle collision with specific object.
	// Cannot be overridden by children; override handleCollisionImpl() instead.
	void handleCollision(SBaseEntity* entity)
	{
		// Execute lambdas (if any) first
		for (auto f : _collisionHandlers)
		{
			f(this, entity);
		}

		handleCollisionImpl(entity);
	};

	void handlePushBack(SBaseEntity* entity)
	{
		// Basic "bumping away" logic
		_collider->handleCollision(entity->getState().get());
	}

	// Initializes state struct to some sane defaults
	virtual void initState(bool generateId = true)
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
	};

	// Rotates the object and all its children. Assumes axis-aligned objects.
	// Value passed defines the new forward vector for the object after
	// rotation.
	virtual void rotate(glm::vec3 orientation)
	{
		glm::vec3 oldRotation = _state->forward;

		_state->forward = orientation;

		// Swap width and depth of collider if necessary
		if (oldRotation.x != _state->forward.x)
		{
			auto temp = _state->width;
			_state->width = _state->depth;
			_state->depth = temp;
		}
	};
		 
	// TODO: add more server-specific functions that are object-agnostic

protected:
	// If any custom collision logic is needed, override this function
	virtual void handleCollisionImpl(SBaseEntity* entity) {};

	// TODO: server-specific state goes here
	std::unique_ptr<BaseCollider> _collider; // bounding box state info
	std::shared_ptr<BaseState> _state;

	// Lambdas to execute inside handleCollision(); lambdas take a pointer
	// to this entity and to the entity that is being collided with. Not
	// using shared pointers because it is not good practice for objects
	// to generate shared pointers from themselves.
	std::vector<std::function<void(SBaseEntity*, SBaseEntity*)>> _collisionHandlers;
};

