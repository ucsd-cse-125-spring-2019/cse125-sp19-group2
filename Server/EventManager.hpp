#pragma once

#include <unordered_map>
#include <vector>
#include <queue>
#include <memory>
#include "Shared/GameEvent.hpp"
#include "Shared/GameState.hpp"
#include "SBaseEntity.hpp"
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
		std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* entityMap,
		NetworkServer* networkInterface,
		std::vector<glm::vec2>* jails,
		std::queue<glm::vec2>* humanSpawns,
		std::queue<glm::vec2>* dogSpawns,
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

	// Raw pointer to global entity map
	std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* _entityMap;

	// Raw pointer to network interface
	NetworkServer* _networkInterface;

	// Jails and player spawn locations
	std::vector<glm::vec2>* _jails;
	std::queue<glm::vec2>* _humanSpawns, * _dogSpawns;

	// Game state struct
	std::shared_ptr<GameState> _gameState;
};

