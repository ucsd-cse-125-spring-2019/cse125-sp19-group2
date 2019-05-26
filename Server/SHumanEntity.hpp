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
	HumanAction _curAction = ACTION_HUMAN_IDLE;

	std::shared_ptr<SPlungerEntity> plungerEntity;
	std::shared_ptr<SRopeEntity> ropeEntity;
	std::function<void()> _launchingReset;

	bool updateAction();

	// Slipping state
	bool _isSlipping = false;
	bool _isSlipImmune = false;
};
