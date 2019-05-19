#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Shared/Logger.hpp"
#include "Shared/QuadTree.hpp"
#include "Shared/GameState.hpp"
#include "NetworkServer.hpp"
#include "SBaseEntity.hpp"
#include "LevelParser.hpp"
#include "CollisionManager.hpp"
#include "EventManager.hpp"
#include "StructureInfo.hpp"

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
	/** Functions **/

	// Update basic game state info (started, inLobby, etc)
	void updateGameState();

	// Update jails' gates y position
	void updateGateHeight();

	/** Variables **/

    // Interface for client communication
    std::unique_ptr<NetworkServer> _networkInterface;

	// Parser for text or JSON level files
	std::unique_ptr<LevelParser> _levelParser;

	// Event handler
	std::unique_ptr<EventManager> _eventManager;

	// Collision handler
	std::unique_ptr<CollisionManager> _collisionManager;

	// Struct to keep track of game state
	std::shared_ptr<GameState> _gameState;

	// Struct containing general game info
	StructureInfo* _structureInfo;
};

