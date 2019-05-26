#include <ctime>
#include <time.h>
#include <random>
#include <glm/glm.hpp>

#include "GridLevelParser.hpp"
#include "SDogEntity.hpp"
#include "SHumanEntity.hpp"
#include "SBoxEntity.hpp"
#include "GameServer.hpp"

GameServer::GameServer()
{
}


GameServer::~GameServer()
{
}


void GameServer::start()
{
    // Start network server
    _networkInterface = std::make_unique<NetworkServer>(PORTNUM);

	// Initialize game state struct
	_gameState = std::make_shared<GameState>();
	_gameState->type = ENTITY_STATE;
	_gameState->gameStarted = false;
	_gameState->gameOver = false;
	_gameState->inLobby = true;
	_gameState->pregameCountdown = false;
	_gameState->millisecondsToStart = 0;
	_gameState->millisecondsLeft = std::chrono::duration_cast<std::chrono::milliseconds>(MAX_GAME_LENGTH).count();
	_gameState->millisecondsToLobby = 0;
	_gameState->dogs = std::unordered_map<uint32_t, std::string>();
	_gameState->humans = std::unordered_map<uint32_t, std::string>();
	_gameState->readyPlayers = std::vector<uint32_t>();

	// Server utilization monitor
	Logger::getInstance()->initUtilizationMonitor();

	// Struct of all useful objects for event manager and otherwise
	_structureInfo = new StructureInfo();

	// Init level parser
	_levelParser = std::make_unique<GridLevelParser>();

	// Map initialization
	_levelParser->parseLevelFromFile("Levels/map.dat", _structureInfo);

	Logger::getInstance()->debug("Parsed " + std::to_string(_structureInfo->entityMap->size()) + " entities from file.");

	// Ensure at least one human spawn, dog spawn, and jail
	// TODO: convert these to exceptions, or find a cleaner way to handle this
	if (!_structureInfo->jails->size())
	{
		Logger::getInstance()->fatal("No jails found in level file");
		fgetc(stdin);
		exit(1);
	}
	if (!_structureInfo->humanSpawns->size())
	{
		Logger::getInstance()->fatal("No human spawn locations found in level file");
		fgetc(stdin);
		exit(1);
	}
	if (!_structureInfo->dogSpawns->size())
	{
		Logger::getInstance()->fatal("No dog spawn locations found in level file");
		fgetc(stdin);
		exit(1);
	}

	// Init collision manager
	_collisionManager = std::make_unique<CollisionManager>(_structureInfo->entityMap);

	// Init event handler
	_eventManager = std::make_unique<EventManager>(
		_networkInterface.get(),
		_structureInfo,
		_gameState);

    // Run update loop, keeping each iteration at a minimum of 1 tick
	while (true)
	{
		auto timerStart = std::chrono::steady_clock::now();

		this->update();

		auto timerEnd = std::chrono::steady_clock::now();

		// Elapsed time in nanoseconds
		auto elapsed = timerEnd - timerStart;

		Logger::getInstance()->storeLoopDuration(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
		//Logger::getInstance()->debug("Elapsed time: " + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()));

		// Warn if server is overloaded
		if (tick(elapsed).count() > 1)
		{
			Logger::getInstance()->warn("Update loop took longer than a tick (" +
					std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()) +
					"ms)");
		}
		else
		{
			// Sleep for the difference
			std::this_thread::sleep_for(tick(1) - elapsed);
		}
	}
}


void GameServer::update()
{
	// General game state and network updates



	// add new entities from last tick to the entity map
	for (auto& newEntity : *_structureInfo->newEntities)
	{
		_structureInfo->entityMap->insert({newEntity->getState()->id, newEntity});
	}
	_structureInfo->newEntities->clear();

	// Handle events from clients and update() each entity
	_eventManager->update();

	// Collision resolution
	_collisionManager->handleCollisions();

	// Update general state of the game based on updates and clock
	updateGameState();

	// Build update list for clients
	auto updates = std::vector<std::shared_ptr<BaseState>>();
	for (auto& entityPair : *_structureInfo->entityMap)
	{
		uint32_t id = entityPair.first;
		std::shared_ptr<SBaseEntity> entity = entityPair.second;

		// Add to vector only if there is an update available
		if (entity->hasChanged)
		{
			updates.push_back(entity->getState());
		}
	}

	// Send out the updates
	_networkInterface->sendUpdates(updates);

	// Send a copy of the GameState struct if any players are connected.
	// This is a bit dangerous because if the client is not receiving anything
	// the network queues could fill up pretty damn fast. It is also mostly a
	// waste of CPU and bandwidth; we may just want the clients to use their
	// own clocks for the countdown.
	if (_gameState->dogs.size() || _gameState->humans.size())
	{
		_networkInterface->sendUpdate(_gameState);
	}

	// Remove all entities marked for deletion
	auto deletedEntities = std::vector<uint32_t>();

	// Build list first, otherwise we would break the iteration
	for (auto& entityPair : *_structureInfo->entityMap)
	{
		if (entityPair.second->getState()->isDestroyed)
		{
			deletedEntities.push_back(entityPair.first);
		}
	}

	// Delete everything in list
	for (auto& id : deletedEntities)
	{
		_structureInfo->entityMap->erase(_structureInfo->entityMap->find(id));
	}
}


void GameServer::updateGameState()
{
	// Check if all the dogs have been caught
	bool dogsCaught = true;
	for (auto& dogPair : _gameState->dogs)
	{
		uint32_t dogId = dogPair.first;
		auto dog = _structureInfo->entityMap->find(dogId);
		if (dog != _structureInfo->entityMap->end())
		{
			dogsCaught &= std::static_pointer_cast<SDogEntity>(dog->second)->isCaught;
		}
	}

	/** Timers **/

	// Increment pregame countdown timer
	if (_gameState->pregameCountdown)
	{
		auto duration = std::chrono::steady_clock::now() - _gameState->_pregameStart;
		_gameState->millisecondsToStart =
			(long)std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::duration_cast<std::chrono::nanoseconds>(PREGAME_LENGTH)
				- duration).count();

		// If new value is zero or negative, start the game
		if (_gameState->millisecondsToStart <= 0)
		{
			_gameState->gameStarted = true;
			_gameState->pregameCountdown = false;
			_gameState->_gameStart = std::chrono::steady_clock::now();
		}
	}

	// Increment game timer if game is started
	if (_gameState->gameStarted)
	{
		_gameState->_gameDuration = std::chrono::steady_clock::now() - _gameState->_gameStart;
		_gameState->millisecondsLeft =
			(long)std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::duration_cast<std::chrono::nanoseconds>(MAX_GAME_LENGTH)
				- _gameState->_gameDuration).count();

		if (_gameState->millisecondsLeft < 0)
		{
			_gameState->millisecondsLeft = 0;
		}
	}

	// Increment postgame countdown timer and reset the game if necessary
	if (_gameState->gameOver)
	{
		auto duration = std::chrono::steady_clock::now() - _gameState->_endgameStart;
		_gameState->millisecondsToLobby =
			(long)std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::duration_cast<std::chrono::nanoseconds>(POSTGAME_LENGTH)
				- duration).count();

		if (_gameState->millisecondsToLobby <= 0)
		{
			_gameState->inLobby = true;
			_gameState->gameOver = false;
			_gameState->readyPlayers.clear();
			_gameState->millisecondsLeft = std::chrono::duration_cast<std::chrono::milliseconds>(MAX_GAME_LENGTH).count();

			// Reset game state
			for (auto& entityPair : *_structureInfo->entityMap)
			{
				// Remove players and pee puddles. TODO: other state reset stuff
				if (entityPair.second->getState()->type == ENTITY_HUMAN ||
					entityPair.second->getState()->type == ENTITY_DOG ||
					entityPair.second->getState()->type == ENTITY_PUDDLE)
				{
					entityPair.second->getState()->isDestroyed = true;
				}
			}
		}
	}

	// Game end logic
	if (_gameState->gameStarted && dogsCaught)
	{
		_gameState->gameStarted = false;
		_gameState->gameOver = true;
		_gameState->winner = ENTITY_HUMAN;
		_gameState->_endgameStart = std::chrono::steady_clock::now();
		Logger::getInstance()->debug("Humans won!");
	}
	else if (_gameState->gameStarted && _gameState->_gameDuration >= MAX_GAME_LENGTH)
	{
		_gameState->gameStarted = false;
		_gameState->gameOver = true;
		_gameState->winner = ENTITY_DOG;
		_gameState->_endgameStart = std::chrono::steady_clock::now();
		Logger::getInstance()->debug("Dogs won!");
	}
}
