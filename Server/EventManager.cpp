#include "EventManager.hpp"
#include "SHumanEntity.hpp"
#include "SDogEntity.hpp"

EventManager::EventManager(
	NetworkServer* networkInterface,
	StructureInfo* structureInfo)
{
	_networkInterface = networkInterface;
	_structureInfo = structureInfo;
	_gameState = structureInfo->gameState.get();
}

EventManager::~EventManager()
{
}


bool EventManager::update()
{
	auto playerEvents = _networkInterface->receiveEvents();

	// Create map of events
	auto eventMap = std::unordered_map<uint32_t, std::vector<std::shared_ptr<GameEvent>>>();

	// Now iterate over player events, inserting them into their respective
	// vectors if they are to be handled by the entities, or handling them
	// here otherwise.
	for (auto& event : playerEvents)
	{
		switch (event->type)
		{
		case EVENT_PLAYER_JOIN:
		{
			handlePlayerJoin(event);
			break;
		}
		case EVENT_PLAYER_SWITCH:
		{
			handlePlayerSwitch(event);
			break;
		}
		case EVENT_PLAYER_LEAVE:
		{
			if (!handlePlayerLeave(event))
			{
				return false;
			}
			break;
		}
		case EVENT_PLAYER_READY:
		{
			handlePlayerReady(event);
			break;
		}
		default:
			// By default, the entity handles the event
			auto it = eventMap.find(event->playerId);
			if (it == eventMap.end())
			{
				eventMap.insert({ event->playerId, std::vector<std::shared_ptr<GameEvent>>() });
			}
			eventMap.find(event->playerId)->second.push_back(event);
		}
	}

	// Call update() on all entities if we are not in the pregame countdown
	if (!_gameState->pregameCountdown)
	{
		for (auto& entityPair : *_structureInfo->entityMap)
		{
			// There are some cases where the map does not have a vector
			auto it = eventMap.find(entityPair.first);
			if (it == eventMap.end())
			{
				eventMap.insert({ entityPair.first, std::vector<std::shared_ptr<GameEvent>>() });
			}

			auto eventVec = eventMap.find(entityPair.first)->second;

			// Sort by event type
			std::sort(eventVec.begin(), eventVec.end(),
				[](const std::shared_ptr<GameEvent> & a, const std::shared_ptr<GameEvent> & b) -> bool
				{
					return a->type < b->type;
				});

			entityPair.second->update(eventVec);
		}
	}

	return true;
}

void EventManager::handlePlayerJoin(std::shared_ptr<GameEvent> event)
{
	// Immediately close the connection if a game is running
	if (!_gameState->inLobby)
	{
		_networkInterface->closePlayerSession(event->playerId);
	}

	// Pick a side for the player
	if (_gameState->dogs.size() <= _gameState->humans.size())
	{
		_gameState->dogs.insert({ event->playerId, event->playerName });
	}
	else
	{
		_gameState->humans.insert({ event->playerId, event->playerName });
	}

	Logger::getInstance()->debug(
		std::string("\"") + event->playerName +
		std::string("\" joined the server!"));
}

void EventManager::handlePlayerSwitch(std::shared_ptr<GameEvent> event)
{
	auto dogsResult = _gameState->dogs.find(event->playerId);
	auto humansResult = _gameState->humans.find(event->playerId);

	if (dogsResult != _gameState->dogs.end())
	{
		_gameState->humans.insert({ event->playerId, dogsResult->second });
		_gameState->dogs.erase(dogsResult);
	}
	else if (humansResult != _gameState->humans.end())
	{
		_gameState->dogs.insert({ event->playerId, humansResult->second });
		_gameState->humans.erase(humansResult);
	}
}

void EventManager::handlePlayerReady(std::shared_ptr<GameEvent> event)
{
	// Disregard duplicate events
	for (auto& player : _gameState->readyPlayers)
	{
		if (player == event->playerId)
		{
			return;
		}
	}

	_gameState->readyPlayers.push_back(event->playerId);

	// Check if everyone is ready
	if (_gameState->dogs.size() + _gameState->humans.size() == _gameState->readyPlayers.size())
	{
		startGame();
	}
}

bool EventManager::handlePlayerLeave(std::shared_ptr<GameEvent> event)
{
	if (_gameState->dogs.find(event->playerId) != _gameState->dogs.end())
	{
		_gameState->dogs.erase(
			_gameState->dogs.find(event->playerId));
	}
	else if (_gameState->humans.find(event->playerId) != _gameState->humans.end())
	{
		_gameState->humans.erase(
			_gameState->humans.find(event->playerId));
	}

	// Remove from ready vector if it exists
	_gameState->readyPlayers.erase(
		std::remove(
			_gameState->readyPlayers.begin(),
			_gameState->readyPlayers.end(),
			event->playerId),
		_gameState->readyPlayers.end());

	// Reset the game if no other players
	if (!_gameState->dogs.size() && !_gameState->humans.size())
	{
		_gameState->inLobby = true;
		_gameState->gameStarted = false;
		_gameState->gameOver = false;
		_gameState->readyPlayers.clear();
		_gameState->millisecondsLeft = std::chrono::duration_cast<std::chrono::milliseconds>(MAX_GAME_LENGTH).count();
		return false;
	}
	
	// If everyone is now ready, start the game
	if (_gameState->inLobby &&
		_gameState->readyPlayers.size() ==
		_gameState->dogs.size() + _gameState->humans.size())
	{
		startGame();
	}

	// Mark entity for deletion if it exists
	auto result = _structureInfo->entityMap->find(event->playerId);
	if (result != _structureInfo->entityMap->end()) {
		auto entity = result->second;
		entity->getState()->isDestroyed = true;
		entity->hasChanged = true;
	}

	return true;
}

void EventManager::startGame()
{
	// Immediately send a new GameState to try and update ready button before
	// the game loads on the client
	_networkInterface->sendUpdate(_structureInfo->gameState);

	// Create human with different skins
	int skinID = 0;

	// Build player entities for each human
	for (auto& humanPair : _gameState->humans)
	{
		// Get first element from human spawn locations and push it back
		glm::vec2 humanSpawn = _structureInfo->humanSpawns->front();
		_structureInfo->humanSpawns->pop();
		_structureInfo->humanSpawns->push(humanSpawn);

		auto humanEntity = std::make_shared<SHumanEntity>(
			humanPair.first, // ID
			humanPair.second, // Name
			_structureInfo,
			skinID);

		// Set location
		humanEntity->getState()->pos = glm::vec3(humanSpawn.x, 0, humanSpawn.y);

		// Insert into global map
		_structureInfo->entityMap->insert({ humanPair.first, humanEntity });

		skinID++;
	}

	// Create dog with different skins
	skinID = 0;

	// Build player entities for each dog
	for (auto& dogPair : _gameState->dogs)
	{
		// Get first element from dog spawn locations and push it back
		glm::vec2 dogSpawn = _structureInfo->dogSpawns->front();
		_structureInfo->dogSpawns->pop();
		_structureInfo->dogSpawns->push(dogSpawn);

		auto dogEntity = std::make_shared<SDogEntity>(
			dogPair.first, // ID
			dogPair.second, // Name
			_structureInfo,
			skinID);

		// Set location
		dogEntity->getState()->pos = glm::vec3(dogSpawn.x, 0, dogSpawn.y);

		// Insert into global map
		_structureInfo->entityMap->insert({ dogPair.first, dogEntity });

		skinID++;
	}

	// Send state of every object to every player
	auto updateVec = std::vector<std::shared_ptr<BaseState>>();
	for (auto& entityPair : *_structureInfo->entityMap)
	{
		updateVec.push_back(entityPair.second->getState());
	}
	_networkInterface->sendUpdates(updateVec);

	_gameState->inLobby = false;

	// Only start the game if at least one dog and human
	// If not, players will still be able to run around in the world
	// for debugging purposes, but there will be no winning or losing
	if (_gameState->dogs.size() && _gameState->humans.size())
	{
		// Game has started!
		_gameState->pregameCountdown = true;
		_gameState->_pregameStart = std::chrono::steady_clock::now();
	}

	// Optimization: remove all tile entities from the server map
	for (auto& entityPair : *_structureInfo->entityMap)
	{
		if (entityPair.second->getState()->type == ENTITY_FLOOR)
		{
			entityPair.second->getState()->isDestroyed = true;
		}
	}
}
