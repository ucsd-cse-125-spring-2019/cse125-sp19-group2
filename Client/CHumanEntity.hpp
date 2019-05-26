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
    };
    ~CHumanEntity() {};

	void updateState(std::shared_ptr<BaseState> state) override {
		// Set generic stuff first
		CPlayerEntity::updateState(state);

		// Set human-specific state variables
		auto currentState = std::static_pointer_cast<HumanState>(_state);
		auto newState = std::static_pointer_cast<HumanState>(state);

		// Animation
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

		// Update HUD
		GuiManager::getInstance().updateCharge(currentState->chargeMeter / MAX_HUMAN_CHARGE);
	}
};

