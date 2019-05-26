#pragma once

#include <random>
#include "SPlayerEntity.hpp"
#include "Shared/DogState.hpp"
#include "SPuddleEntity.hpp"

class SDogEntity : public SPlayerEntity
{
public:
	SDogEntity(
		uint32_t playerId,
		std::string playerName,
		StructureInfo* structureInfo);

	~SDogEntity() {};

	bool isCaught = false;

	void update(std::vector<std::shared_ptr<GameEvent>> events) override;

	void generalHandleCollision(SBaseEntity* entity) override;

	bool isInteracting() {
		return _isInteracting;
	}

	void setNearTrigger(bool val) {
		_nearTrigger = val;
	}

	void setNearFountain(bool val) {
		_nearFountain = val;
	}

private:
	int type = 0;

	// _isSomething is just player's attempt of doing it, may not be the final action
	bool _isUrinating = false;
	bool _isRunning = false;
	bool _isInteracting = false;

	DogAction _curAction = ACTION_DOG_IDLE;

	// State for jail triggers
	bool _nearTrigger = false;

	// State for fountains
	bool _nearFountain = false;

	// Dog pee timer
	Timer* _peeTimer;

	void createPuddle();

	// decide what action the player is actually in (not including action caused by external reason)
	bool updateAction();

};

