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
		humanAnimation->play(newState->currentAnimation);

		// Human attributes and items
		// TODO
	}
};

