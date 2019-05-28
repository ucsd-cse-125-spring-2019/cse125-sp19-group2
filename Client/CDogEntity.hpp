#pragma once

#include "CPlayerEntity.hpp"
#include "Shared/DogState.hpp"
#include "Shared/Logger.hpp"

class CDogEntity : public CPlayerEntity
{
public:
    CDogEntity(uint32_t id) : CPlayerEntity()
    {
		// Init player state
		_state = std::make_shared<DogState>();

        // Load Animation
        initAnimation("./Resources/Models/dog.dae");

		// Load positional sounds
		_pantingSound = AudioManager::getInstance().getAudioSource("dog panting" + std::to_string(id));
		_pantingSound->init("Resources/Sounds/dog_panting.wav", true, true);
		_pantingSound->setVolume(0.1f);
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

		// Panting
		_pantingSound->play(newState->currentAnimation == ANIMATION_DOG_IDLE);

		// Animation
		currentState->currentAnimation = newState->currentAnimation;

		// Requires a cast
		Animation* dogAnimation = static_cast<Animation*>(_objectModel.get());
		if (newState->isPlayOnce)
		{
			dogAnimation->playOnce(newState->currentAnimation, newState->animationDuration);
		}
		else
		{
			dogAnimation->play(newState->currentAnimation);
		}

		// Dog attributes and items
		currentState->runStamina = newState->runStamina;
		currentState->urineMeter = newState->urineMeter;

		// Update HUD
		GuiManager::getInstance().updateStamina(currentState->runStamina / MAX_DOG_STAMINA);
		GuiManager::getInstance().updatePee(currentState->urineMeter / MAX_DOG_URINE);
	}

protected:
	AudioSource * _pantingSound;
};

