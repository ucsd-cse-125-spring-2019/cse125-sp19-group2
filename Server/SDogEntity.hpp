#pragma once

#include <random>
#include "SPlayerEntity.hpp"
#include "Shared/DogState.hpp"
#include "SPuddleEntity.hpp"

#define BASE_VELOCITY 5.0f
#define RUN_VELOCITY BASE_VELOCITY * 1.5f;

class SDogEntity : public SPlayerEntity
{
public:
	SDogEntity(
		uint32_t playerId,
		std::string playerName,
		std::vector<glm::vec2>* jails,
		std::vector<std::shared_ptr<SBaseEntity>>* newEntities);

	~SDogEntity() {};

	bool isCaught = false;

	void update(std::vector<std::shared_ptr<GameEvent>> events) override;

	void generalHandleCollision(SBaseEntity* entity) override;

	bool isLifting() {
		return _isLifting;
	}

	void setNearTrigger(bool val) {
		_nearTrigger = val;
	}


	// dirty booleans
	int actionStage = 0;

private:
	// List of jails the dog could potentially be sent to
	std::vector<glm::vec2>* _jails;
	int type = 0;
	// _isSomething is just player's attempt of doing it, may not be the final action
	bool _isUrinating = false;
	bool _isRunning = false;
	bool _isLifting = false;
	bool _isDrinking = false;
	bool _nearTrigger = false;
	DogAction _curAction = ACTION_DOG_IDLE;
	std::chrono::time_point<std::chrono::system_clock> _urinatingStartTime;
	std::vector<std::shared_ptr<SBaseEntity>>* _newEntities;

	void createPuddle();

	// decide what action the player is actually in (not including action caused by external reason)
	bool updateAction();

	
};

