#include "ParticleSystemManager.hpp"
#include "UrineParticleSystem.hpp"
#include "Shared/DogState.hpp"

ParticleSystemManager & ParticleSystemManager::getInstance()
{
	static ParticleSystemManager particleSystemManager;
	return particleSystemManager;
}

void ParticleSystemManager::clear()
{
	_systemsMap.clear();
}

ParticleSystemManager::ParticleSystemManager()
{
}

void ParticleSystemManager::updatePhysics(float delta_time)
{
	for (auto system : _systemsMap) system.second->Update(delta_time);
}

void ParticleSystemManager::updateState(std::shared_ptr<BaseState> const & state)
{
	if (state->isDestroyed)
	{
		erase(state->id);
		return;
	}

	if (state->type == ENTITY_DOG)
	{
		// Dog state and urine particle system
		auto dogState = std::static_pointer_cast<DogState>(state);
		std::shared_ptr<UrineParticleSystem> urineSystem = nullptr;

		// Check if particle system exists
		auto result = _systemsMap.find(dogState->id);
		if (result != _systemsMap.end())
		{
			urineSystem = std::static_pointer_cast<UrineParticleSystem>(result->second);
		}
		else
		{
			urineSystem = std::static_pointer_cast<UrineParticleSystem>(createSystem(ParticleSystemType::URINE));
			_systemsMap.insert({ dogState->id, urineSystem });
		}

		// Update particle system parameters
		if (dogState->currentAnimation == DogAnimation::ANIMATION_DOG_PEEING)
		{
			urineSystem->set_is_urinating(true);
			urineSystem->set_origin(dogState->pos + glm::vec3(0.0f, 0.1f, 0.0f));
			urineSystem->set_velocity(dogState->forward * 0.2f);
			Logger::getInstance()->info("Urinating...");
		}
		else
		{
			urineSystem->set_is_urinating(false);
			Logger::getInstance()->info("Stopping urination...");
		}
	}
}

void ParticleSystemManager::render(std::unique_ptr<Camera> const & camera)
{
	for (auto system : _systemsMap) system.second->Draw(camera);
}

std::shared_ptr<ParticleSystem> ParticleSystemManager::createSystem(ParticleSystemType system_type)
{
	std::shared_ptr<ParticleSystem> system = nullptr;

	switch (system_type)
	{
	case ParticleSystemType::URINE:
		auto urineSystem = std::make_shared<UrineParticleSystem>();
		urineSystem->set_texture("urine.png");
		urineSystem->set_rate(100.0f);
		urineSystem->set_lifespan(2.0f);
		urineSystem->set_size(0.03f, 0.03f);
		urineSystem->set_mass(0.3f);

		system = urineSystem;
		break;
	}

	// Return the particle system that was added
	return system;
}

void ParticleSystemManager::erase(uint32_t id)
{
	auto result = _systemsMap.find(id);
	if (result != _systemsMap.end()) _systemsMap.erase(result);
}
