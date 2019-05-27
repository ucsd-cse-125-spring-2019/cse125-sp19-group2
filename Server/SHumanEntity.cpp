#include "SHumanEntity.hpp"

SHumanEntity::SHumanEntity(
	uint32_t playerId,
	std::string playerName,
	StructureInfo* structureInfo)
{
	_structureInfo = structureInfo;

	_state = std::make_shared<HumanState>();

	// Parent initialization
	SPlayerEntity::initState();
	_state->id = playerId;
	_state->type = ENTITY_HUMAN;

	// Collider stuff
	_collider = std::make_unique<CapsuleCollider>(_state.get());
	_state->colliderType = COLLIDER_CAPSULE;
	_state->width = 0.9f;
	_state->height = 2.0f;
	_state->depth = 0.9f;

	_velocity = HUMAN_BASE_VELOCITY;

	// Human-specific stuff
	auto humanState = std::static_pointer_cast<HumanState>(_state);
	humanState->currentAnimation = ANIMATION_HUMAN_IDLE;

	// Player-specific stuff
	humanState->playerName = playerName;

	_launchingReset = [&] {
		_isLaunching = false;
		if (plungerEntity != nullptr)
		{
			plungerEntity->getState()->isDestroyed = true;
			plungerEntity = nullptr;
		}
		if (ropeEntity != nullptr)
		{
			ropeEntity->getState()->isDestroyed = true;
			ropeEntity = nullptr;
		}
	};

	_swingingReset = [&] {
		if (netEntity != nullptr)
		{
			netEntity->getState()->isDestroyed = true;
			netEntity = nullptr;
			_netDistance = 0;
			_netVelocity = 0;
		}
	};
}

void SHumanEntity::update(std::vector<std::shared_ptr<GameEvent>> events)
{
	auto humanState = std::static_pointer_cast<HumanState>(_state);

	// Non-movement events, duplicates removed
	auto filteredEvents = SPlayerEntity::getFilteredEvents(events);

	for (auto& event : filteredEvents)
	{
		switch (event->type)
		{
		case EVENT_PLAYER_CHARGE_NET:
			if (!_isSwinging)
			{
				_isCharging = true;
			}
			else
			{
				_isCharging = false;
			}
			break;
		case EVENT_PLAYER_SWING_NET:
			_isCharging = false;
			_isSwinging = true;
			break;
		case EVENT_PLAYER_LAUNCH_START:
			_isLaunching = true;
			_plungerDirection = glm::vec3(event->direction.x, 0, event->direction.y);
			break;
		case EVENT_PLAYER_LAUNCH_END:
			_isLaunching = false;
			if (plungerEntity != nullptr)
			{
				_launchingReset();

				// stop human from flying
				if (actionStage == 3) {
					_isInterpolating = false;

					// Run onInterrupt lambda if it exists
					if (_interpOnInterrupt)
					{
						_interpOnInterrupt();
					}
				}
			}
			
			break;
		}
	}

	// Save old position
	glm::vec3 oldPos = _state->pos;

	// Update and check for changes
	SPlayerEntity::update(events);

	// Update net charge amount
	if (_isCharging && humanState->chargeMeter < MAX_HUMAN_CHARGE) {
		humanState->chargeMeter += 2.0f / TICKS_PER_SEC;
		hasChanged = true;
	}

	bool actionChanged = updateAction();

	// Reset stage of action if changed to a new action
	if (actionChanged) {
		actionStage = 0;
	}

	switch (_curAction)
	{
	case ACTION_HUMAN_IDLE:
		if (actionChanged) {
			humanState->currentAnimation = ANIMATION_HUMAN_IDLE;
			hasChanged = true;
		}
		break;
	case ACTION_HUMAN_MOVING:
		if (actionChanged) {
			humanState->currentAnimation = ANIMATION_HUMAN_RUNNING;
			hasChanged = true;
		}
		handleActionMoving();
		break;
	case ACTION_HUMAN_LAUNCHING:
		if (actionChanged) {
			_state->forward = _plungerDirection;
			humanState->currentAnimation = ANIMATION_HUMAN_SHOOT;
			hasChanged = true;
			// Timer until shooting animation end
			registerTimer(360, [&]()
			{
				humanState->currentAnimation = ANIMATION_HUMAN_IDLE_LAUNCHER;
				hasChanged = true;
				actionStage++;
			});
		}

		// stage 1: create plunger entity and wait until it hit the wall
		if (actionStage == 1) {
			if (plungerEntity == nullptr) {
				plungerEntity = std::make_shared<SPlungerEntity>(_state->pos, _state->forward);
				_structureInfo->newEntities->push_back(plungerEntity);
			}
			if (ropeEntity == nullptr) {
				ropeEntity = std::make_shared<SRopeEntity>();
				_structureInfo->newEntities->push_back(ropeEntity);
			}
			if (!plungerEntity->launching) {
				actionStage++;
			}
		}

		// stage 2: human fly to plunger
		if (actionStage == 2) {
			humanState->currentAnimation = ANIMATION_HUMAN_FLYING;
			hasChanged = true;
			glm::vec3 plungerTailPos = plungerEntity->getState()->pos + glm::normalize(plungerEntity->getState()->forward) * -0.675f;
			interpolateMovement(plungerTailPos, plungerEntity->getState()->forward, 20.0f,
				_launchingReset, _launchingReset, false);
			actionStage++;
		}

		// update rope start point and end point
		if (ropeEntity != nullptr)
		{
			glm::vec3 gunPos = _state->pos + glm::normalize(_state->forward) * 0.7f;
			glm::vec3 plungerTailPos = plungerEntity->getState()->pos + glm::normalize(plungerEntity->getState()->forward) * -0.675f;
			ropeEntity->updatePoints(gunPos, plungerTailPos);
		}

		break;
	case ACTION_HUMAN_SLIPPING:
		if (actionChanged) {
			humanState->currentAnimation = ANIMATION_HUMAN_SLIPPING;
			humanState->isPlayOnce = true;
			humanState->animationDuration = 1500;
			hasChanged = true;

			// Timer until immobility stops
			registerTimer(2000, [&]()
				{
					_isSlipping = false;
					_isSlipImmune = true;
					hasChanged = true;

					// Disable slipping for another two seconds after
					// initial slip
					registerTimer(2000, [&]()
						{
							_isSlipImmune = false;
							hasChanged = true;
						});
				});
		}
		break;
	case ACTION_HUMAN_SWINGING:
		if (actionChanged) {

			float chargeDistance = humanState->chargeMeter * 3.0f;
			float chargeDuration = chargeDistance / HUMAN_SWING_VELOCITY;
			stuntDuration = (chargeDistance / HUMAN_BASE_VELOCITY - chargeDistance / HUMAN_SWING_VELOCITY + 0.2f) * 1250;

			if (netEntity == nullptr)
			{
				netEntity = std::make_shared<SCylinderEntity>(_state->pos, glm::vec3(0.5f, 1.0f, 0.5f));
				netEntity->getState()->isSolid = false;
				netEntity->hasChanged = true;
				_structureInfo->newEntities->push_back(netEntity);
			}

			// animation based on how high is the charge meter
			if (humanState->chargeMeter < HUMAN_CHARGE_THRESHOLD1)
			{
				// swing1->idle min time: 19/60
				stuntDuration = std::max(stuntDuration, (19.0f / 60 - chargeDuration) * 1000);
				humanState->currentAnimation = ANIMATION_HUMAN_SWINGING1;
				humanState->isPlayOnce = true;
				humanState->animationDuration = stuntDuration + chargeDuration;
				_netVelocity = 0.1f;
				_netMaxDistance = 1.0f;
			}
			else if (humanState->chargeMeter < HUMAN_CHARGE_THRESHOLD2)
			{
				// swing2->idle min time: 36/60
				stuntDuration = std::max(stuntDuration, (36.0f / 60 - chargeDuration) * 1000);
				humanState->currentAnimation = ANIMATION_HUMAN_SWINGING2;
				humanState->isPlayOnce = true;
				humanState->animationDuration = stuntDuration + chargeDuration;
				_netVelocity = 0.08f;
				_netMaxDistance = 2.0f;
			}
			else
			{
				// swing3->idle min time: 55/60
				stuntDuration = std::max(stuntDuration, (55.0f / 60 - chargeDuration) * 1000);
				humanState->currentAnimation = ANIMATION_HUMAN_SWINGING3;
				humanState->isPlayOnce = true;
				humanState->animationDuration = stuntDuration + chargeDuration;
				_netVelocity = 0.08f;
				_netMaxDistance = 2.0f;
			}
				

			hasChanged = true;

			// alarm for end of charging
			registerTimer(chargeDuration * 1000, [&]()
			{
				actionStage++;

				// alarm for end of stunt
				registerTimer(stuntDuration, [&]()
				{
					_isSwinging = false;
					humanState->chargeMeter = 0;
					hasChanged = true;
					_swingingReset();
				});
			});
		}

		// stage 0: human moving forward and net moving forward
		if (actionStage == 0) {
			_state->pos += _state->forward * (HUMAN_SWING_VELOCITY / TICKS_PER_SEC);
			hasChanged = true;
		}

		if (netEntity != nullptr)
		{
			_netDistance = std::min(_netDistance + _netVelocity, _netMaxDistance);
			netEntity->getState()->pos = _state->pos + _state->forward * _netDistance;
			netEntity->hasChanged = true;
		}

		break;
	}

	handleInterpolation();
	//Logger::getInstance()->debug("Human   x: " + std::to_string(_state->forward.x) + " y: " + std::to_string(_state->forward.y) + " z: " + std::to_string(_state->forward.z));
}

void SHumanEntity::generalHandleCollision(SBaseEntity * entity)
{
	// Base collision handling first
	SPlayerEntity::generalHandleCollision(entity);

	// Player slipping in puddle
	if (entity->getState()->type == ENTITY_PUDDLE &&
		!_isSlipImmune)
	{
		// Allow human to step on edges without slipping
		if (glm::length(entity->getState()->pos - _state->pos) <
			(0.75 * entity->getState()->width / 2))
		{
			_isSlipping = true;
		}
	}

	// Cast for player-specific stuff
	auto humanState = std::static_pointer_cast<HumanState>(_state);

}

bool SHumanEntity::updateAction()
{
	HumanAction oldAction = _curAction;

	// lower the priority of action if possible
	if (_curAction == ACTION_HUMAN_LAUNCHING && !_isLaunching ||
		_curAction == ACTION_HUMAN_SLIPPING && !_isSlipping ||
		_curAction == ACTION_HUMAN_SWINGING && !_isSwinging)
	{
		_curAction = ACTION_HUMAN_IDLE;
	}

	// all other actions has higher priority than idle and moving
	if (_curAction == ACTION_HUMAN_IDLE || _curAction == ACTION_HUMAN_MOVING) {
		// change action based on attempting to move or not
		_curAction = (_isMoving) ? ACTION_HUMAN_MOVING : ACTION_HUMAN_IDLE;

		// update action again if higher priority action is happening
		if (_isLaunching) _curAction = ACTION_HUMAN_LAUNCHING;
		else if (_isSlipping && !_isSlipImmune) _curAction = ACTION_HUMAN_SLIPPING;
		else if (_isSwinging) _curAction = ACTION_HUMAN_SWINGING;
	}

	// if failed to swing then cancel swing
	if (!_curAction == ACTION_HUMAN_SWINGING)
	{
		_isSwinging = false;
	}

	return oldAction != _curAction;
}
