#pragma once

#include <memory>
#include <glm/gtx/string_cast.hpp>

#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/QuadTree.hpp"
#include "Shared/Timer.hpp"
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

	virtual ~SBaseEntity();	// Destroys local state and collider objects

	// Update function, called every tick. Override if additional functionality
	// is desired
	virtual void update(std::vector<std::shared_ptr<GameEvent>> events) {};

    // All server objects must have a state to send to the client.
	virtual std::shared_ptr<BaseState> getState();

	// For objects with "scene graphs". By default, no kids
	virtual	std::vector<std::shared_ptr<SBaseEntity>> getChildren();

	// Wrappers for colliders
	virtual bool isColliding(QuadTree & tree);

	virtual bool isColliding(BaseState* state);

	virtual std::vector<BaseState*> getColliding(QuadTree & tree);

	// Registers custom collision handler for this object. Called inside
	// handleCollision()
	template<typename T>
	void onCollision(T && lambda)
	{
		_collisionHandlers.push_back(lambda);
	}

	// Called by the CollisionManager; handle collision with specific object.
	// Cannot be overridden by children; override generalHandleCollision() instead.
	void handleCollision(SBaseEntity* entity);

	// Basic "bumping away" logic
	void handlePushBack(SBaseEntity* entity);

	// Initializes state struct to some sane defaults. Be sure to call on
	// any children
	virtual void initState(bool generateId = true);

	// Registers a timer with the entity
	Timer * registerTimer(long durationMilliseconds, std::function<void()> f);

	// Updates existing timers. This must be manually called every tick in
	// order for timers to work!
	void updateTimers();

	// Rotates the object and all its children around a point.
	// Angle is the number of times to be rotated clockwise in
	// 90 degree steps.
	virtual void rotate(glm::vec3 center, int angle);
		 
	// TODO: add more server-specific functions that are object-agnostic

protected:
	// If any custom collision logic is needed, override this function
	virtual void generalHandleCollision(SBaseEntity* entity) {};

	// TODO: server-specific state goes here
	std::unique_ptr<BaseCollider> _collider; // bounding box state info
	std::shared_ptr<BaseState> _state;

	// Lambdas to execute inside handleCollision(); lambdas take a pointer
	// to this entity and to the entity that is being collided with. Not
	// using shared pointers because it is not good practice for objects
	// to generate shared pointers from themselves.
	std::vector<std::function<void(SBaseEntity*, SBaseEntity*)>> _collisionHandlers;

private:
	// Helper function to rotate forward vector 90 degrees clockwise
	glm::vec3 rotateOnce(glm::vec3 vec);

	// Timers to fire events
	std::vector<Timer*> _timers;
};
