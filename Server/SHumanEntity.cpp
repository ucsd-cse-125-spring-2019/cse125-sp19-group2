#include "SHumanEntity.hpp"

SHumanEntity::SHumanEntity(uint32_t playerId, std::string playerName, std::vector<std::shared_ptr<SBaseEntity>>* newEntities)
{
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

	_velocity = 4.8f;

	_newEntities = newEntities;

	// Human-specific stuff
	auto humanState = std::static_pointer_cast<HumanState>(_state);
	humanState->currentAnimation = ANIMATION_HUMAN_IDLE;

	// Player-specific stuff
	humanState->playerName = playerName;
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
		case EVENT_PLAYER_SWING_NET:
			// Example of lunging. Will probably need to change
			interpolateMovement(_state->pos + (_state->forward * 1.5f), _state->forward, 15.0f);
			break;
		case EVENT_PLAYER_LAUNCH_START:
			_isLaunching = true;
			break;
		case EVENT_PLAYER_LAUNCH_END:
			_isLaunching = false;
			plungerEntity->getState()->isDestroyed = true;

			plungerEntity = nullptr;
			break;
		}
	}

	// Save old position
	glm::vec3 oldPos = _state->pos;

	// Update and check for changes
	SPlayerEntity::update(events);

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
			humanState->currentAnimation = ANIMATION_HUMAN_SHOOT;
			hasChanged = true;
		}

		if (plungerEntity == nullptr) {
			plungerEntity = std::make_shared<SPlungerEntity>(_state->pos, _state->forward);
			_newEntities->push_back(plungerEntity);
		}

		break;
	case ACTION_HUMAN_SLIPPING:
		if (actionChanged) {
			humanState->currentAnimation = ANIMATION_HUMAN_SLIPPING;
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
	}



	handleInterpolation();
	//Logger::getInstance()->debug("Human   x: " + std::to_string(_state->forward.x) + " y: " + std::to_string(_state->forward.y) + " z: " + std::to_string(_state->forward.z));

	// Reset slipping state
	_isSlipping = false;
}

void SHumanEntity::generalHandleCollision(SBaseEntity * entity)
{
	// Base collision handling first
	SPlayerEntity::generalHandleCollision(entity);

	// Player slipping in puddle
	if (entity->getState()->type == ENTITY_PUDDLE)
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
		_curAction == ACTION_HUMAN_SLIPPING && !_isSlipping)
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
	}

	return oldAction != _curAction;
}
