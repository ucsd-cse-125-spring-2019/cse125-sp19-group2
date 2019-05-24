#pragma once

#include "SPlayerEntity.hpp"
#include "Shared/HumanState.hpp"
#include "SPlungerEntity.hpp"

class SHumanEntity : public SPlayerEntity
{
public:
	SHumanEntity(uint32_t playerId,
		std::string playerName,
		std::vector<std::shared_ptr<SBaseEntity>>* newEntities);

	~SHumanEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events) override;

	void generalHandleCollision(SBaseEntity* entity) override;

private:
	bool _isLaunching = false;
	HumanAction _curAction = ACTION_HUMAN_IDLE;
	std::vector<std::shared_ptr<SBaseEntity>>* _newEntities;

	std::shared_ptr<SPlungerEntity> plungerEntity;

	bool updateAction();

	// Slipping state
	bool _isSlipping = false;
	bool _isSlipImmune = false;
};
