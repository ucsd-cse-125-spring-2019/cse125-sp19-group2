#pragma once

#include "CPlayerEntity.hpp"
#include "Shared/DogState.hpp"

class CDogEntity : public CPlayerEntity
{
public:
    CDogEntity() : CPlayerEntity()
    {
		// Init player state
		_state = std::make_shared<DogState>();

        // Load Animation
        initAnimation("./Resources/Models/dog.dae");
    };
    ~CDogEntity() {};

	void updateState(std::shared_ptr<BaseState> state) override {
		// Set generic stuff first
		CPlayerEntity::updateState(state);

		// Set dog-specific state variables
		auto currentState = std::static_pointer_cast<DogState>(_state);
		auto newState = std::static_pointer_cast<DogState>(state);
		
		// Animation
		currentState->currentAnimation = newState->currentAnimation;
		_playerModel->animatedMesh->_takeIndex = newState->currentAnimation;

		// Dog attributes and items
		currentState->runStamina = newState->runStamina;
	}
};

