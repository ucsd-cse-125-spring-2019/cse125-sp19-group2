#pragma once

#include "CPlayerEntity.hpp"
#include "Shared/DogState.hpp"
#include "UrineParticleSystem.hpp"
#include "Shared/Logger.hpp"

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
		auto urineSystem = std::static_pointer_cast<UrineParticleSystem>(_particleSystems[UrineSystemIndex]);
		urineSystem->set_texture("urine.png");
		urineSystem->set_rate(20.0f);
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

		// Emit new urine particles (if in urinating state)
		auto urineSystem = std::static_pointer_cast<UrineParticleSystem>(_particleSystems[UrineSystemIndex]);
		if (currentState->currentAnimation == DogAnimation::ANIMATION_DOG_PEEING)
		{
			urineSystem->set_is_urinating(true);
			urineSystem->set_origin(_state->pos + glm::vec3(0.0f, 3.0f, 0.0f));
			urineSystem->set_velocity(_state->forward * 0.3f);
			Logger::getInstance()->info("Urinating...");
		}
		else
		{
			urineSystem->set_is_urinating(false);
		}

		// Update urine particle system physics
		urineSystem->Update(0.016f);

		Logger::getInstance()->info("Updating...");
	}
private:
	const unsigned int UrineSystemIndex = 0;
};

