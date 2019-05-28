#pragma once

#include "CPlayerEntity.hpp"
#include "Shared/HumanState.hpp"

class CHumanEntity : public CPlayerEntity
{
public:
    CHumanEntity(uint32_t id) : CPlayerEntity()
    {
		// Init player state
		_state = std::make_shared<HumanState>();

        // Load Animation
        initAnimation("./Resources/Models/human.dae");

		// Load positional sounds
		_runningSound = AudioManager::getInstance().getAudioSource("human running" + std::to_string(id));
		_runningSound->init("Resources/Sounds/human_running.wav", true, true);
		_runningSound->setVolume(0.1f);

		_swingSound = AudioManager::getInstance().getAudioSource("human swinging" + std::to_string(id));

		_flyingSound = AudioManager::getInstance().getAudioSource("human flying" + std::to_string(id));
		_flyingSound->init("Resources/Sounds/human_flying.wav", true, true);
		_flyingSound->setVolume(0.1f);

		_slippingSound = AudioManager::getInstance().getAudioSource("human slipping" + std::to_string(id));

    };
    ~CHumanEntity() {};

	void updateState(std::shared_ptr<BaseState> state) override {
		// Set generic stuff first
		CPlayerEntity::updateState(state);

		// Set human-specific state variables
		auto currentState = std::static_pointer_cast<HumanState>(_state);
		auto newState = std::static_pointer_cast<HumanState>(state);

		/** Sounds **/
		_runningSound->setPosition(_state->pos);
		_swingSound->setPosition(_state->pos);
		_flyingSound->setPosition(_state->pos);
		_slippingSound->setPosition(_state->pos);

		// Running
		_runningSound->play(newState->currentAnimation == ANIMATION_HUMAN_RUNNING);

		// Net swing
		if (newState->currentAnimation != currentState->currentAnimation &&
			(newState->currentAnimation == ANIMATION_HUMAN_SWINGING1 ||
			 newState->currentAnimation == ANIMATION_HUMAN_SWINGING2 ||
			 newState->currentAnimation == ANIMATION_HUMAN_SWINGING3))
		{
			_swingSound->init("Resources/Sounds/swing1.wav", false, true);
			_swingSound->setVolume(0.3f);
			_swingSound->setPosition(_state->pos);
			_swingSound->play(true);
		}

		// Flying
		_flyingSound->play(newState->currentAnimation == ANIMATION_HUMAN_FLYING);

		// Slipping
		if (newState->currentAnimation != currentState->currentAnimation &&
			newState->currentAnimation == ANIMATION_HUMAN_SLIPPING)
		{
			_slippingSound->init("Resources/Sounds/human_slipping.wav", false, true);
			_slippingSound->setVolume(0.10f);
			_slippingSound->setPosition(_state->pos);
			_slippingSound->play(true);
		}

		/** Animation **/
		currentState->currentAnimation = newState->currentAnimation;

		// Requires a cast
		Animation* humanAnimation = static_cast<Animation*>(_objectModel.get());
		if (newState->isPlayOnce)
		{
			humanAnimation->playOnce(newState->currentAnimation, newState->animationDuration);
		}
		else
		{
			humanAnimation->play(newState->currentAnimation);
		}

		// Human attributes and items
		currentState->chargeMeter = newState->chargeMeter;

		if (_isLocal)
		{
			// Update HUD
			GuiManager::getInstance().updatePlunger(newState->plungerCooldown);
			GuiManager::getInstance().updateTrap(newState->trapCooldown);
			GuiManager::getInstance().updateCharge(currentState->chargeMeter / MAX_HUMAN_CHARGE);
		}
	}

protected:
	AudioSource* _runningSound;
	AudioSource* _swingSound;
	AudioSource* _flyingSound;
	AudioSource* _slippingSound;
};

