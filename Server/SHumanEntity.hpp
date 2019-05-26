#pragma once

#include "SPlayerEntity.hpp"
#include "Shared/HumanState.hpp"
#include "SPlungerEntity.hpp"
#include "SRopeEntity.hpp"

class SHumanEntity : public SPlayerEntity
{
public:
	SHumanEntity(uint32_t playerId,
		std::string playerName,
		StructureInfo* structureInfo);

	~SHumanEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events) override;

	void generalHandleCollision(SBaseEntity* entity) override;

private:
	bool _isLaunching = false;
	bool _isPlacingTrap = false;
	HumanAction _curAction = ACTION_HUMAN_IDLE;

	std::shared_ptr<SPlungerEntity> plungerEntity;
	std::shared_ptr<SRopeEntity> ropeEntity;
	std::function<void()> _launchingReset;

	glm::vec3 _plungerDirection;

	bool updateAction();

	void updateCooldowns();

	// Cooldowns
	std::chrono::time_point<std::chrono::steady_clock> _plungerCooldownStart;
	std::chrono::time_point<std::chrono::steady_clock> _trapCooldownStart;

	// Slipping state
	bool _isSlipping = false;
	bool _isSlipImmune = false;
	bool _isCharging = false;
	bool _isSwinging = false;
	float stuntDuration = 0;
};
