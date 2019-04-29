#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Shared/Logger.hpp"
#include "NetworkServer.hpp"
#include "SBaseEntity.hpp"
#include "Shared/QuadTree.hpp"

using tick = std::chrono::duration<double, std::ratio<1, TICKS_PER_SEC>>;

// Max game length
const std::chrono::seconds MAX_GAME_LENGTH(30);

struct PairHash;	// Forward declaration

class GameServer
{
public:
	GameServer();
	~GameServer();

    // Setup components and start main loop
    void start();

    // Main update loop
    void update();

private:
	/** Variables **/

    // Interface for client communication
    std::unique_ptr<NetworkServer> _networkInterface;

    // Map of all game entities
    std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>> _entityMap;

	// Game is currently started (not in lobby mode). Note: game starts in
	// lobby mode
	bool _gameStarted = false;

	// Game over message
	bool _gameOver = false;

	// List of dogs and humans
	std::vector<std::shared_ptr<SBaseEntity>> _dogs;
	std::vector<std::shared_ptr<SBaseEntity>> _humans;

	// Timer for game itself
	std::chrono::nanoseconds _gameDuration;

	/** Functions **/

	// Resolve collisions
	void handleCollisions();

	// AABB collision resolution
	void handleAABB(BaseState* stateA, BaseState* stateB);

	// Capsule collision resolution
	void handleCapsule(BaseState* stateA, BaseState* stateB);

	// Dog-human collisions
	void handleDogCaught(
		BaseState* dog,
		BaseState* human,
		std::unordered_set<std::pair<BaseState*, BaseState*>, PairHash> & collisionSet,
		QuadTree * tree);
};

