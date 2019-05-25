#pragma once

#include "CPlayerEntity.hpp"
#include "Shared/DogState.hpp"
#include "UrineParticleSystem.hpp"

class CDogEntity : public CPlayerEntity
{
public:
	CDogEntity() : CPlayerEntity()
	{
		// Init player state
		_state = std::make_shared<DogState>();

		// Load Animation
		initAnimation("./Resources/Models/dog.dae");

		// Add urine particle system
		_particleSystems.push_back(std::make_shared<UrineParticleSystem>());
		std::dynamic_pointer_cast<UrineParticleSystem>(_particleSystems[UrineSystemIndex])->set_texture("urine.png");
		std::dynamic_pointer_cast<UrineParticleSystem>(_particleSystems[UrineSystemIndex])->set_rate(20.0f);
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

		// Requires a cast
		Animation* dogAnimation = static_cast<Animation*>(_objectModel.get());
		dogAnimation->animatedMesh->_takeIndex = newState->currentAnimation;

		// Dog attributes and items
		currentState->runStamina = newState->runStamina;

		// Update urine particle system
		std::dynamic_pointer_cast<UrineParticleSystem>(_particleSystems[UrineSystemIndex])->set_origin(_state->pos);
		std::dynamic_pointer_cast<UrineParticleSystem>(_particleSystems[UrineSystemIndex])->set_velocity(_state->forward * 0.3f);
		std::dynamic_pointer_cast<UrineParticleSystem>(_particleSystems[UrineSystemIndex])->Update(0.016f);
	}
private:
	const unsigned int UrineSystemIndex = 0;
};

