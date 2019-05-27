#pragma once

#include "ParticleSystem.hpp"
#include "Camera.hpp"
#include "Shared/BaseState.hpp"
#include "Shared/Logger.hpp"

enum class ParticleSystemType : unsigned char
{
	URINE,
	FOUNTAIN
};

class ParticleSystemManager
{
public:

	static ParticleSystemManager& getInstance();

	/**
	 * \brief Perform physics-based particle system update.
	 */
	void updatePhysics(float delta_time);

	/**
	 * \brief Dispatch new state to the entity to update particle system
	 * \param state(std::shared_ptr<BaseState> const&) Update state
	 */
	void updateState(std::shared_ptr<BaseState> const &state);

	/**
	 * \brief Render all particle systems in the game world
	 * \param camera(std::unique_ptr<Camera> const&) Local camera
	 */
	void render(std::unique_ptr<Camera> const &camera);

	std::shared_ptr<ParticleSystem> createSystem(ParticleSystemType system_type);

	void erase(uint32_t id);

	/**
	 * \brief Remove all particle systems in the world
	 */
	void clear();

private:

	ParticleSystemManager();

	std::unordered_map<uint32_t, std::shared_ptr<ParticleSystem>> _systemsMap;
};