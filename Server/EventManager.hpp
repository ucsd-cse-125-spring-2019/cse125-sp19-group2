#pragma once

#include <unordered_map>
#include <vector>
#include <queue>
#include <memory>
#include "Shared/GameEvent.hpp"
#include "Shared/GameState.hpp"
#include "SBaseEntity.hpp"
#include "StructureInfo.hpp"
#include "NetworkServer.hpp"

/*
** This class gets events from the clients, filters them, and
** calls the update() function on each entity on the server with the
** appropriate events.
*/
class EventManager
{
public:
	EventManager(
		NetworkServer* networkInterface,
		StructureInfo* structureInfo,
		std::shared_ptr<GameState> gameState);
	~EventManager();

	// Update all entities. Gets events from clients and calls update() on
	// every entity on the server.
	void update();

private:
	// Helper functions
	void handlePlayerJoin(std::shared_ptr<GameEvent> event);
	void handlePlayerSwitch(std::shared_ptr<GameEvent> event);
	void handlePlayerReady(std::shared_ptr<GameEvent> event);
	void handlePlayerLeave(std::shared_ptr<GameEvent> event);

	// Raw pointer to network interface
	NetworkServer* _networkInterface;

	// General structure info
	StructureInfo* _structureInfo;

	// Game state struct
	std::shared_ptr<GameState> _gameState;
};

