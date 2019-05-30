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
		StructureInfo* structureInfo);
	~EventManager();

	// Update all entities. Gets events from clients and calls update() on
	// every entity on the server. Returns false if players left and the
	// server is now empty, true otherwise.
	//
	// Having it return a bool is not the cleanest way of handling this but
	// I don't want to make a GameStateManager just to handle this one case.
	bool update();

private:
	// Helper functions
	void handlePlayerJoin(std::shared_ptr<GameEvent> event);
	void handlePlayerSwitch(std::shared_ptr<GameEvent> event);
	void handlePlayerReady(std::shared_ptr<GameEvent> event);

	// Handle player leave events. Returns true if there are still players
	// on the server, false otherwise.
	bool handlePlayerLeave(std::shared_ptr<GameEvent> event);

	void handleResendRequest(std::shared_ptr<GameEvent> event);

	void startGame();

	// Raw pointer to network interface
	NetworkServer* _networkInterface;

	// General structure info
	StructureInfo* _structureInfo;

	// Game state struct. It is contained within _structureInfo; we keep this
	// variable as an alias for convenience.
	GameState* _gameState;
};

