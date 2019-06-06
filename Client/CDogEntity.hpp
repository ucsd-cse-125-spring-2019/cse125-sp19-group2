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

		_drinkingSound = AudioManager::getInstance().getAudioSource("dog drinking" + std::to_string(id));
		_drinkingSound->init("Resources/Sounds/dog_drinking.wav", true, true);
		_drinkingSound->setVolume(0.3f);

		_peeingSound = AudioManager::getInstance().getAudioSource("dog peeing" + std::to_string(id));
		_peeingSound->init("Resources/Sounds/dog_peeing.wav", true, true);
		_peeingSound->setVolume(0.1f);
    };
    ~CDogEntity() {};

	void updateState(std::shared_ptr<BaseState> state) override {
		// Set generic stuff first
		CPlayerEntity::updateState(state);

		// Set dog-specific state variables
		auto currentState = std::static_pointer_cast<DogState>(_state);
		auto newState = std::static_pointer_cast<DogState>(state);
		
		/** Sounds **/
		if (!_yelpingSound)
		{
			_yelpingSound = AudioManager::getInstance().getAudioSource("dog yelping" + std::to_string(_state->id));
		}

		if (!_barkingSound)
		{
			_barkingSound = AudioManager::getInstance().getAudioSource("dog barking" + std::to_string(_state->id));
		}

		_pantingSound->setPosition(_state->pos);
		_runningSound->setPosition(_state->pos);
		_drinkingSound->setPosition(_state->pos);
		_peeingSound->setPosition(_state->pos);
		_yelpingSound->setPosition(_state->pos);
		_barkingSound->setPosition(_state->pos);

		// Panting
		_pantingSound->play(newState->currentAnimation == ANIMATION_DOG_IDLE);

		// Running
		_runningSound->play(newState->currentAnimation == ANIMATION_DOG_RUNNING);

		// Drinking
		_drinkingSound->play(newState->currentAnimation == ANIMATION_DOG_DRINKING);

		// Peeing
		_peeingSound->play(newState->currentAnimation == ANIMATION_DOG_PEEING);

		// Yelping when caught
		if (!currentState->isCaught && newState->isCaught)
		{
			_yelpingSound->init("Resources/Sounds/dog_yelping.wav", false, true);
			_yelpingSound->setVolume(0.3f);
			_yelpingSound->play(true);
		}

		// Barking on interact button
		if (!currentState->isBarking && newState->isBarking)
		{
			/* Disabled for now, creates some strange artifacts*/

			//_barkingSound->init("Resources/Sounds/dog_barking.wav", false, true);
			//_barkingSound->setVolume(0.3f);
			//_barkingSound->play(true);
		}

		currentState->isCaught = newState->isCaught;
		currentState->isBarking = newState->isBarking;

		/** Animation **/
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
		if (_isLocal)
		{
			currentState->runStamina = newState->runStamina;
			currentState->urineMeter = newState->urineMeter;

			// Update HUD
			GuiManager::getInstance().updateStamina(currentState->runStamina / MAX_DOG_STAMINA);
			GuiManager::getInstance().updatePee(currentState->urineMeter / MAX_DOG_URINE);
		}
	}

	bool isCaught() {
		auto currentState = std::static_pointer_cast<DogState>(_state);
		return currentState->isCaught;
	}

protected:
	AudioSource * _pantingSound;
	AudioSource * _runningSound;
	AudioSource * _drinkingSound;
	AudioSource * _peeingSound;
	AudioSource * _yelpingSound = nullptr;
	AudioSource * _barkingSound = nullptr;
};

