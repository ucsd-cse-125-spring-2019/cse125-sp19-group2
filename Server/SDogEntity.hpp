#pragma once

#include <random>
#include "SPlayerEntity.hpp"
#include "Shared/DogState.hpp"
#include "SPuddleEntity.hpp"
#include "STrapEntity.hpp"

class SDogEntity : public SPlayerEntity
{
public:
	SDogEntity(
		uint32_t playerId,
		std::string playerName,
		StructureInfo* structureInfo,
		int skinID);

	~SDogEntity() {};

	bool isCaught = false;
	bool isTeleporting = false;

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

	void setTeleporting(bool val) {
		_isTeleporting = val;
	}

	void setSourceDoghouseDir(glm::vec3 dir) {
		_sourceDoghouseDir = dir;
	}

	void setTargetDoghousePos(glm::vec3 pos) {
		_targetDoghousePos = pos;
	}

	void setTargetDoghouseDir(glm::vec3 dir) {
		_targetDoghouseDir = dir;
	}

private:
	int type = 0;

	// _isSomething is just player's attempt of doing it, may not be the final action
	bool _isUrinating = false;
	bool _isRunning = false;
	bool _isInteracting = false;
	bool _isTrapped = false;
	bool _isJailed = false;	// Teleporting to jail
	bool _isTeleporting = false;	// Teleporting to a doghouse

	// Jail teleportation stuff
	glm::vec3 _targetJailPos;

	// Doghouse teleportation stuff
	glm::vec3 _sourceDoghouseDir;
	glm::vec3 _targetDoghousePos;
	glm::vec3 _targetDoghouseDir;

	// Number of times the dog has pressed a button to escape trap
	int _numEscapePressed = 0;

	// If the dog is currently trapped, this points to the trap
	SBaseEntity* _curTrap;

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

