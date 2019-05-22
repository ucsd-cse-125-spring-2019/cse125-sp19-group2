#pragma once

#include "SPlayerEntity.hpp"
#include "Shared/HumanState.hpp"

class SHumanEntity : public SPlayerEntity
{
public:
	SHumanEntity(uint32_t playerId, std::string playerName);;

	~SHumanEntity() {};

	void update(std::vector<std::shared_ptr<GameEvent>> events) override;

	void generalHandleCollision(SBaseEntity* entity) override;

private:
	HumanAction _curAction = ACTION_HUMAN_IDLE;

	bool updateAction();
};

