#pragma once

#include "CPlayerEntity.hpp"
#include "Shared/DogState.hpp"
#include "Shared/Logger.hpp"

class CDogEntity : public CPlayerEntity
{
public:
    CDogEntity(uint32_t id, int skinID) : CPlayerEntity()
    {
		// Init player state
		_state = std::make_shared<DogState>();

		// Load Animation based on skin
		std::string modelLoc = "./Resources/Models/dog";
		int usedSkin = skinID % DOG_SKIN_AMOUNT;
		if (usedSkin != 0)
			modelLoc += std::to_string(usedSkin);
		modelLoc += ".dae";
        initAnimation(modelLoc);

		// Load positional sounds
		_pantingSound = AudioManager::getInstance().getAudioSource("dog panting" + std::to_string(id));
		_pantingSound->init("Resources/Sounds/dog_panting.wav", true, true);
		_pantingSound->setVolume(0.1f);

		_runningSound = AudioManager::getInstance().getAudioSource("dog running" + std::to_string(id));
		_runningSound->init("Resources/Sounds/dog_running.wav", true, true);
		_runningSound->setVolume(0.2f);
    };
    ~CDogEntity() {};

	void updateState(std::shared_ptr<BaseState> state) override {
		// Set generic stuff first
		CPlayerEntity::updateState(state);

		// Set dog-specific state variables
		auto currentState = std::static_pointer_cast<DogState>(_state);
		auto newState = std::static_pointer_cast<DogState>(state);
		
		/** Sounds **/
		_pantingSound->setPosition(_state->pos);
		_runningSound->setPosition(_state->pos);

		// Panting
		_pantingSound->play(newState->currentAnimation == ANIMATION_DOG_IDLE);
		_runningSound->play(newState->currentAnimation == ANIMATION_DOG_RUNNING);

		// Animation
		currentState->currentAnimation = newState->currentAnimation;

		// Requires a cast
		Animation* dogAnimation = static_cast<Animation*>(_objectModel.get());
		if (newState->isPlayOnce)
		{
			dogAnimation->playOnce(dogAnimations[newState->currentAnimation], newState->animationDuration);
		}
		else
		{
			dogAnimation->play(dogAnimations[newState->currentAnimation]);
		}

		// Dog attributes and items
		if (_isLocal)
		{
			currentState->runStamina = newState->runStamina;
			currentState->urineMeter = newState->urineMeter;

			// Update HUD
			GuiManager::getInstance().updateStamina(currentState->runStamina / MAX_DOG_STAMINA);
			GuiManager::getInstance().updatePee(currentState->urineMeter / MAX_DOG_URINE);
		}
	}

protected:
	AudioSource * _pantingSound;
	AudioSource * _runningSound;
};

