#pragma once

#include "CPlayerEntity.hpp"
#include "Shared/HumanState.hpp"

class CHumanEntity : public CPlayerEntity
{
public:
    CHumanEntity() : CPlayerEntity()
    {
		// Init player state
		_state = std::make_shared<HumanState>();

        // Load Animation
        initAnimation("./Resources/Models/human.dae");

		_runningSound = AudioManager::getInstance().getAudioSource("human running");
		_runningSound->init("Resources/Sounds/human_running.wav", true, true);
		_runningSound->setVolume(0.1f);

		_swingSound = AudioManager::getInstance().getAudioSource("human swinging");
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
			_swingSound->play(true);
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
};

