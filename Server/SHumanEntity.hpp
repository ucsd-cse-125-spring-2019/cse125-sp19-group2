#pragma once

#include "SPlayerEntity.hpp"
#include "Shared/HumanState.hpp"
#include "SPlungerEntity.hpp"
#include "SRopeEntity.hpp"
#include "SCylinderEntity.hpp"

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
	std::shared_ptr<SCylinderEntity> netEntity;
	std::function<void()> _launchingReset;
	std::function<void()> _swingingReset;

	float _netDistance = 0;
	float _netVelocity = 0;
	float _netMaxDistance = 0;

	glm::vec3 _plungerDirection;

	bool updateAction();

	// Slipping state
	bool _isSlipping = false;
	bool _isSlipImmune = false;
	bool _isCharging = false;
	bool _isSwinging = false;
	float stuntDuration = 0;
};
