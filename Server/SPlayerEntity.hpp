#pragma once

#include "IdGenerator.hpp"
#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"
#include "StructureInfo.hpp"
#include "Shared/PlayerState.hpp"
#include <algorithm>

// Amount of leway when comparing floats
const float FP_EPSILON = 0.00001f;

class SPlayerEntity : public SBaseEntity
{
public:
	SPlayerEntity() {};

	~SPlayerEntity() {};

	virtual void initState(bool generateId = true) override
	{
		// Base init
		SBaseEntity::initState(false);

		// Players are not static
		_state->isStatic = false;
	}

	virtual void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		auto playerState = std::static_pointer_cast<PlayerState>(_state);

		// Do nothing if we are set to be destroyed
		if (_state->isDestroyed)
		{
			return;
		}

		hasChanged = false;

		// Only change attributes of this object if not static
		if (!_state->isStatic)
		{
			// Movement events
			std::vector<std::shared_ptr<GameEvent>> movementEvents;
			std::copy_if(events.begin(), events.end(), std::back_inserter(movementEvents), [&](std::shared_ptr<GameEvent> i)
			{
				return i->type == EVENT_PLAYER_MOVE;
			});

			// Movement logic
			if (movementEvents.size())
			{
				if (!_isInterpolating)
				{
					// Sort by vector
					std::sort(movementEvents.begin(), movementEvents.end(),
						[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
						{
							if (std::abs(a->direction.x - b->direction.x) < FP_EPSILON)
								return a->direction.x < b->direction.x;
							else
								return a->direction.y < b->direction.y;
						});

					// Remove duplicate vectors
					movementEvents.erase(std::unique(movementEvents.begin(), movementEvents.end(),
						[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
						{
							return a->direction == b->direction;
						}), movementEvents.end());

					// Overall direction of player; take average of all direction vectors
					glm::vec3 dir = glm::vec3(0);

					for (auto& moveEvent : movementEvents)
					{
						dir += glm::vec3(moveEvent->direction.x, 0, moveEvent->direction.y);
					}

					// Update forward vector with unit direction only if it was modified
					if (glm::length(dir) > 0.0001f)
					{
						_isMoving = true;
						_newDir = glm::normalize(dir);
					}
					else {
						_isMoving = false;
					}
				} // !_isInterpolating
			} // player movement

			// Player look events
			std::vector<std::shared_ptr<GameEvent>> lookEvents;
			std::copy_if(events.begin(), events.end(), std::back_inserter(lookEvents), [&](std::shared_ptr<GameEvent> i)
			{
				return i->type == EVENT_PLAYER_LOOK;
			});

			// Set player forward to last value
			if (lookEvents.size() && !_lookDirLocked)
			{
				auto direction = glm::normalize(lookEvents.back()->direction);
				_state->forward = glm::vec3(direction.x, 0, direction.y);
				hasChanged = true;
			}

			// Check for player stop event
			for (auto& event : events)
			{
				if (event->type == EVENT_PLAYER_STOP)
				{
					_isMoving = false;
				}
			}
		} // isStatic

		// Update all timers for the player
		updateTimers();

		// Reset custom player message
		playerState->message = "";

	} // update()

	// Call this function to move a player to a destination
	void interpolateMovement(
		glm::vec3 destination,
		glm::vec3 finalDirection,
		float velocity,
		std::function<void()> onComplete = 0,
		std::function<void()> onInterrupt = 0) 
	{
		if (!_isInterpolating)
		{
			_isInterpolating = true;
			_destination = destination;
			_finalDirection = finalDirection;
			_interpVelocity = velocity;
			_interpOnComplete = onComplete;
			_interpOnInterrupt = onInterrupt;
		}
	}

	// Stage of current action
	int actionStage = 0;

	// Used when interpolating to a colliding object
	glm::vec3 targetPos;
	glm::vec3 targetDir;

protected:
	// General server state info
	StructureInfo* _structureInfo;

	// Player movement velocity in units/second
	float _velocity;
	glm::vec3 _newDir;

	// For the case in which client FPS is lower than tick rate
	bool _isMoving = false;

	// Whether the player can freely look around
	bool _lookDirLocked = false;

	// Interpolation stuff
	bool _isInterpolating = false;
	glm::vec3 _destination;
	glm::vec3 _finalDirection;
	float _interpVelocity; // Interpolation velocity, in units/sec

	// Function to be called when interpolation is complete
	std::function<void()> _interpOnComplete;

	// Function to be called when interpolation is interrupted
	std::function<void()> _interpOnInterrupt;

	// Called by update() every tick
	void handleInterpolation()
	{
		if (_isInterpolating)
		{
			// Vector to final position
			glm::vec3 diff = _destination - _state->pos;
			glm::vec3 unitDiff = diff / glm::length(diff);

			if (glm::length(diff) == 0)
			{
				_isInterpolating = false;
				_state->forward = _finalDirection;
				hasChanged = true;
				if (_interpOnComplete)
				{
					_interpOnComplete();
				}
				return;
			}

			glm::vec3 movementVec = unitDiff * (_interpVelocity / TICKS_PER_SEC);

			if (glm::length(movementVec) > glm::length(diff))
			{
				_state->pos = _destination;
				_state->forward = _finalDirection;
				_isInterpolating = false;
				if (_interpOnComplete)
				{
					_interpOnComplete();
				}
			}
			else
			{
				_state->pos += movementVec;
				_state->forward = unitDiff;
			}
			hasChanged = true;
		}
	}

	void generalHandleCollision(SBaseEntity* entity) override
	{
		// If interpolating and we hit a solid object, stop
		if (_isInterpolating && entity->getState()->getSolidity(_state.get()))
		{
			_isInterpolating = false;

			// Run onInterrupt lambda if it exists
			if (_interpOnInterrupt)
			{
				_interpOnInterrupt();
			}
		}
	}

	// Filters out movement and duplicate events
	std::vector<std::shared_ptr<GameEvent>> getFilteredEvents(
		std::vector<std::shared_ptr<GameEvent>> events)
	{
		// Filter for non-movement events
		auto filteredEvents = std::vector<std::shared_ptr<GameEvent>>();
		for (auto& event : events)
		{
			if (event->type != EVENT_PLAYER_MOVE &&
				event->type != EVENT_PLAYER_LOOK)
			{
				filteredEvents.push_back(event);
			}
		}

		// Remove duplicates
		filteredEvents.erase(std::unique(filteredEvents.begin(), filteredEvents.end(),
			[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
		{
			return a->type == b->type;
		}), filteredEvents.end());

		return filteredEvents;
	}

	void handleActionMoving() {
		_state->forward = _newDir;
		// Move player by (direction * velocity) / ticks_per_sec
		_state->pos = _state->pos + ((_state->forward * _velocity) / (float)TICKS_PER_SEC);
		hasChanged = true;
	}
};

