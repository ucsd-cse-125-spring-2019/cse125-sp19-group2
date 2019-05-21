#include "EventManager.hpp"
#include "SHumanEntity.hpp"
#include "SDogEntity.hpp"

EventManager::EventManager(
	std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* entityMap,
	std::vector<std::shared_ptr<SBaseEntity>>* newEntities,
	NetworkServer* networkInterface,
	std::vector<glm::vec2>* jails,
	std::queue<glm::vec2>* humanSpawns,
	std::queue<glm::vec2>* dogSpawns,
	std::shared_ptr<GameState> gameState)
{
	_entityMap = entityMap;
	_newEntities = newEntities;
	_networkInterface = networkInterface;
	_jails = jails;
	_humanSpawns = humanSpawns;
	_dogSpawns = dogSpawns;
	_gameState = gameState;
}

EventManager::~EventManager()
{
}


void EventManager::update()
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
			handlePlayerLeave(event);
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

	// Call update() on all entities
	for (auto& entityPair : *_entityMap)
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
	_gameState->numReady += 1;

	// Immediately send a new GameState
	_networkInterface->sendUpdate(_gameState);

	// Check if everyone is ready
	if (_gameState->dogs.size() + _gameState->humans.size() == _gameState->numReady)
	{
		// Build player entities for each human
		for (auto& humanPair : _gameState->humans)
		{
			// Get first element from human spawn locations and push it back
			glm::vec2 humanSpawn = _humanSpawns->front();
			_humanSpawns->pop();
			_humanSpawns->push(humanSpawn);

			auto humanEntity = std::make_shared<SHumanEntity>(
				humanPair.first,
				humanPair.second);

			// Set location
			humanEntity->getState()->pos = glm::vec3(humanSpawn.x, 0, humanSpawn.y);

			// Insert into global map
			_entityMap->insert({ humanPair.first, humanEntity });
		}

		// Build player entities for each dog
		for (auto& dogPair : _gameState->dogs)
		{
			// Get first element from dog spawn locations and push it back
			glm::vec2 dogSpawn = _dogSpawns->front();
			_dogSpawns->pop();
			_dogSpawns->push(dogSpawn);

			auto dogEntity = std::make_shared<SDogEntity>(
				dogPair.first,
				dogPair.second,
				_jails,
				_newEntities);

			// Set location
			dogEntity->getState()->pos = glm::vec3(dogSpawn.x, 0, dogSpawn.y);

			// Insert into global map
			_entityMap->insert({ dogPair.first, dogEntity });
		}

		// Send state of every object to every player
		auto updateVec = std::vector<std::shared_ptr<BaseState>>();
		for (auto& entityPair : *_entityMap)
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
			Logger::getInstance()->debug("Game started!");
			_gameState->gameStarted = true;
			_gameState->_gameStart = std::chrono::steady_clock::now();
		}
	}
}

void EventManager::handlePlayerLeave(std::shared_ptr<GameEvent> event)
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

	if (_gameState->inLobby)
	{
		_gameState->numReady -= 1;
	}

	// Reset the game if no other players
	if (!_gameState->dogs.size() && !_gameState->humans.size())
	{
		_gameState->inLobby = true;
		_gameState->gameStarted = false;
		_gameState->gameOver = false;
		_gameState->numReady = 0;
	}

	// Mark entity for deletion if it exists
	auto result = _entityMap->find(event->playerId);
	if (result != _entityMap->end()) {
		auto entity = result->second;
		entity->getState()->isDestroyed = true;
		entity->hasChanged = true;
	}
}
