#include "SDogEntity.hpp"

SDogEntity::SDogEntity(uint32_t playerId, std::string playerName, std::vector<glm::vec2>* jails, std::vector<std::shared_ptr<SBaseEntity>>* newEntities)
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
	dogState->urineMeter = MAX_DOG_URINE;

	// Player-specific stuff
	dogState->playerName = playerName;
}

void SDogEntity::update(std::vector<std::shared_ptr<GameEvent>> events)
{
	isCaught = false;
	if (!_isInteracting)
	{
		_nearTrigger = false;
		_nearFountain = false;
	}

	auto dogState = std::static_pointer_cast<DogState>(_state);

	// Refill a little stamina
	if (dogState->runStamina < MAX_DOG_STAMINA)
	{
		// Four seconds to charge 1 second of sprinting
		dogState->runStamina += 1.0f / 4 / TICKS_PER_SEC;
		hasChanged = true;
	}
	else
	{
		dogState->runStamina = MAX_DOG_STAMINA;
	}

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
				break;
			case EVENT_PLAYER_URINATE_START:
				_isUrinating = true;
				break;
			case EVENT_PLAYER_URINATE_END:
				_isUrinating = false;

				// Abort timer if it exists
				if (_peeTimer)
				{
					_peeTimer->abort();
					_peeTimer = nullptr;
				}
				break;
			case EVENT_PLAYER_INTERACT_START:
				_isInteracting = true;
				break;
			case EVENT_PLAYER_INTERACT_END:
				_isInteracting = false;
			default:
				break;
			}
		}
	}

	// Update and check for changes (destroyed and _isMoving)
	SPlayerEntity::update(events);

	// update current action
	bool actionChanged = updateAction();

	// Reset stage of action if changed to a new action
	if (actionChanged) {
		actionStage = 0;
	}

	switch (_curAction)
	{
	case ACTION_DOG_IDLE:
		if (actionChanged) {
			dogState->currentAnimation = ANIMATION_DOG_IDLE;
			hasChanged = true;
		}
		break;
	case ACTION_DOG_MOVING:
		if (actionChanged) {
			dogState->currentAnimation = ANIMATION_DOG_RUNNING;
			hasChanged = true;
		}
		if (_isRunning) {
			if (dogState->runStamina >= 1.0f / TICKS_PER_SEC)
			{
				_velocity = RUN_VELOCITY;
				dogState->runStamina -= 1.0f / TICKS_PER_SEC;
			}
			else {
				dogState->runStamina = 0;
				_velocity = BASE_VELOCITY;
			}
		}
		else {
			_velocity = BASE_VELOCITY;
		}
		handleActionMoving();
		break;
	case ACTION_DOG_PEEING:
		if (actionChanged) {
			dogState->currentAnimation = ANIMATION_DOG_PEEING;
			hasChanged = true;

			// Register a timer and place the pee object after 1 second
			_peeTimer = registerTimer(1000 /* Milliseconds */, [&]()
				{
					if (_curAction == ACTION_DOG_PEEING)
					{
						createPuddle();
						dogState->urineMeter = 0.0f;
						_isUrinating = false;
						_peeTimer = nullptr;
					}
				});
		}
		break;
	case ACTION_DOG_SCRATCHING:
		// Stage 0: interpolating to the trigger and look at it
		if (actionStage == 0) {
			targetPos.y = 0;
			glm::vec3 dest = targetPos + glm::normalize(_state->pos - targetPos) * 0.55f;
			dogState->currentAnimation = ANIMATION_DOG_RUNNING;
			interpolateMovement(dest, glm::normalize(targetPos - _state->pos), BASE_VELOCITY / 2,
				[&] {
				// Stage 1: start scratching animation and lifting the gate
				actionStage++;
				dogState->currentAnimation = ANIMATION_DOG_SCRATCHING;
				hasChanged = true;
			});
		}
		break;
	case ACTION_DOG_DRINKING:
		// Stage 0: interpolating to the fountain and look at it
		if (actionStage == 0) {
			dogState->currentAnimation = ANIMATION_DOG_RUNNING;
			interpolateMovement(targetPos, targetDir, BASE_VELOCITY / 2, [&]()
				{
					// Stage 1: start drinking animation and filling meter
					actionStage++;
					dogState->currentAnimation = ANIMATION_DOG_DRINKING;
					hasChanged = true;
				});
		}
		else if (actionStage == 1) {
			// Increase pee meter
			if (dogState->urineMeter < MAX_DOG_URINE)
			{
				dogState->urineMeter += MAX_DOG_URINE / 3 / TICKS_PER_SEC; // Refilled in 3 seconds
			}
			else
			{
				dogState->urineMeter = MAX_DOG_URINE;
			}
		}
		break;
	}

	handleInterpolation();
}

void SDogEntity::generalHandleCollision(SBaseEntity * entity)
{
	// Player handler first
	SPlayerEntity::generalHandleCollision(entity);

	// Cast for dog-specific stuff
	auto dogState = std::static_pointer_cast<DogState>(_state);

	// Dog getting caught is handled by the dog, not the human
	if (entity->getState()->type == ENTITY_HUMAN && !isCaught)
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

void SDogEntity::createPuddle()
{
	std::shared_ptr<SPuddleEntity> puddleEntity = std::make_shared<SPuddleEntity>(_state->pos);
	_newEntities->push_back(puddleEntity);
}

bool SDogEntity::updateAction()
{
	// Cast for dog-specific stuff
	auto dogState = std::static_pointer_cast<DogState>(_state);

	DogAction oldAction = _curAction;

	// lower the priority of action if possible
	if (_curAction == ACTION_DOG_PEEING && !_isUrinating ||
		_curAction == ACTION_DOG_DRINKING && !_isInteracting ||
		_curAction == ACTION_DOG_SCRATCHING && !_isInteracting)
	{
		_curAction = ACTION_DOG_IDLE;
	}

	// all other actions has higher priority than idle and moving
	if (_curAction == ACTION_DOG_IDLE || _curAction == ACTION_DOG_MOVING) {
		// change action based on attempting to move or not
		_curAction = (_isMoving) ? ACTION_DOG_MOVING : ACTION_DOG_IDLE;

		// update action again if higher priority action is happening
		if (_isUrinating && dogState->urineMeter == 1.0f) _curAction = ACTION_DOG_PEEING;
		else if (_isInteracting && _nearTrigger) _curAction = ACTION_DOG_SCRATCHING;
		else if (_isInteracting && _nearFountain) _curAction = ACTION_DOG_DRINKING;
	}

	return oldAction != _curAction;
}
