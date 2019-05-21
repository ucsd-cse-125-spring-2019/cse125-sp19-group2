#pragma once

#include <random>
#include "SPlayerEntity.hpp"
#include "Shared/DogState.hpp"
#include "SPuddleEntity.hpp"

#define BASE_VELOCITY 5.0f
#define RUN_VELOCITY BASE_VELOCITY * 1.5f;

class SDogEntity : public SPlayerEntity
{
public:
	SDogEntity(
		uint32_t playerId,
		std::string playerName,
		std::vector<glm::vec2>* jails,
    std::vector<std::shared_ptr<SBaseEntity>>* newEntities)
	{
		_state = std::make_shared<DogState>();

		// Parent initialization
		SPlayerEntity::initState();
		_state->id = playerId;
		_state->type = ENTITY_DOG;

		// Collider stuff
		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;
		_state->width = 0.8f;
		_state->height = 0.8f;
		_state->depth = 0.8f;

		_velocity = BASE_VELOCITY;

		_jails = jails;

		_newEntities = newEntities;

		// Dog-specific stuff
		auto dogState = std::static_pointer_cast<DogState>(_state);
		dogState->currentAnimation = ANIMATION_DOG_IDLE;
		dogState->runStamina = MAX_DOG_STAMINA;

		// Player-specific stuff
		dogState->playerName = playerName;
	};

	~SDogEntity() {};

	bool isCaught = false;

	void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		isCaught = false;
		auto dogState = std::static_pointer_cast<DogState>(_state);
		
		// Save old position
		glm::vec3 oldPos = _state->pos;

		// Non-movement events, duplicates removed
		auto filteredEvents = SPlayerEntity::getFilteredEvents(events);

		// If any events left, process them
		if (filteredEvents.size())
		{
			for (auto& event : filteredEvents)
			{
				switch (event->type)
				{
				case EVENT_PLAYER_RUN_START:
					_isRunning = true;
					break;
				case EVENT_PLAYER_RUN_END:
					_isRunning = false;
					_velocity = BASE_VELOCITY;
					break;
				case EVENT_PLAYER_URINATE_START:
					_urinatingStartTime = std::chrono::system_clock::now();
					_isUrinating = true;
					break;
				case EVENT_PLAYER_URINATE_END:
					_isUrinating = false;
					break;
				case EVENT_PLAYER_LIFTING_START:
					_isLifting = true;
					break;
				case EVENT_PLAYER_LIFTING_END:
					_isLifting = false;
				// TODO: event for when player releases running button
				default:
					break;
				}
			}
		}

		if (_isRunning)
		{
			if (dogState->runStamina > 0)
			{
				_velocity = RUN_VELOCITY;
			}
			dogState->runStamina -= 1 / TICKS_PER_SEC;

			if (dogState->runStamina < 0)
			{
				dogState->runStamina = 0;
			}
		}

		if (_isUrinating)
		{
			auto now = std::chrono::system_clock::now();
			std::chrono::duration<double> diff = now - _urinatingStartTime;
			if (diff.count() > 1.0)
			{
				createPuddle();
				_isUrinating = false;
			}
		}

		// Update and check for changes
		if (!(_isUrinating || _isLifting))
			SPlayerEntity::update(events);


		// Set running/not running based on position
		// also, if you start moving while peeing, cancel the puddle
		if (_state->pos != oldPos)
		{
			_isUrinating = false;
			dogState->currentAnimation = ANIMATION_DOG_RUNNING;
		}

		// Check for stop event
		for (auto& event : events)
		{
			if (event->type == EVENT_PLAYER_STOP)
			{
				dogState->currentAnimation = ANIMATION_DOG_IDLE;
				hasChanged = true;
				break;
			}
		}
	}

	void generalHandleCollision(SBaseEntity* entity) override
	{
		// Player handler first
		SPlayerEntity::handleCollisionImpl(entity);

		// Cast for dog-specific stuff
		auto dogState = std::static_pointer_cast<DogState>(_state);

		// Dog getting caught is handled by the dog, not the human
		if (entity->getState()->type == ENTITY_HUMAN)
		{
			// Teleport to a random jail
			unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
			std::shuffle(_jails->begin(), _jails->end(), std::default_random_engine(seed));
			glm::vec2 jailPos = (*_jails)[0];
			getState()->pos = glm::vec3(jailPos.x, 0, jailPos.y);
		}
		else if (entity->getState()->type == ENTITY_BONE)
		{
			// Refill dog stamina
			dogState->runStamina = MAX_DOG_STAMINA;
			hasChanged = true;

			// Remove dog bone
			entity->getState()->isDestroyed = true;
			entity->hasChanged = true;
		}
	}

	bool isLifting() {
		return _isLifting;
	}

private:
	// List of jails the dog could potentially be sent to
	std::vector<glm::vec2>* _jails;
	int type = 0;
	bool _isUrinating = false;
	bool _isRunning = false;
	bool _isLifting = false;
	std::chrono::time_point<std::chrono::system_clock> _urinatingStartTime;
	std::vector<std::shared_ptr<SBaseEntity>>* _newEntities;

	void createPuddle()
	{
		std::shared_ptr<SPuddleEntity> puddleEntity = std::make_shared<SPuddleEntity>(_state->pos);
		_newEntities->push_back(puddleEntity);
	}
};

