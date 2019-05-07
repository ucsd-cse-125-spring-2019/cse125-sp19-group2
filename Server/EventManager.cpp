#include "EventManager.hpp"
#include "SHumanEntity.hpp"
#include "SDogEntity.hpp"

EventManager::EventManager(
	std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* entityMap,
	NetworkServer* networkInterface,
	std::vector<glm::vec2>* jails,
	std::queue<glm::vec2>* humanSpawns,
	std::queue<glm::vec2>* dogSpawns,
	std::shared_ptr<GameState> gameState)
{
	_entityMap = entityMap;
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
		case EVENT_PLAYER_LEAVE:
		{
			handlePlayerLeave(event);
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
	Logger::getInstance()->debug(
		std::string("\"") + event->playerName +
		std::string("\" joined the server!"));

	std::shared_ptr<SBaseEntity> playerEntity;

	// Make player entity; for now, even numbers are human, odd are dogs
	if (event->playerId % 2)
	{
		// Get first element from human spawn locations and push it back
		glm::vec2 humanSpawn = _humanSpawns->front();
		_humanSpawns->pop();
		_humanSpawns->push(humanSpawn);

		// Create player entity at position
		playerEntity = std::make_shared<SHumanEntity>(event->playerId);
		playerEntity->getState()->pos = glm::vec3(humanSpawn.x, 0, humanSpawn.y);
		_gameState->humans.push_back(playerEntity->getState()->id);
	}
	else
	{
		// Get first element from dog spawn locations and push it back
		glm::vec2 dogSpawn = _dogSpawns->front();
		_dogSpawns->pop();
		_dogSpawns->push(dogSpawn);

		// Create player entity at position
		playerEntity = std::make_shared<SDogEntity>(event->playerId, _jails);
		playerEntity->getState()->pos = glm::vec3(dogSpawn.x, 0, dogSpawn.y);
		_gameState->dogs.push_back(playerEntity->getState()->id);
	}

	// Throw it into server-wide map
	_entityMap->insert(std::pair<uint32_t,
		std::shared_ptr<SBaseEntity>>(
		playerEntity->getState()->id, playerEntity));

	// Generate a vector of all object states
	auto updateVec = std::vector<std::shared_ptr<BaseState>>();
	for (auto& entityPair : *_entityMap)
	{
		updateVec.push_back(entityPair.second->getState());
	}

	// Send updates to this player only
	_networkInterface->sendUpdates(updateVec, event->playerId);

	// Also send this player entity to everyone. Results in a
	// duplicate update for the player who joined, but is cleaner
	// than a boolean check inside update()
	_networkInterface->sendUpdate(playerEntity->getState());
}

void EventManager::handlePlayerLeave(std::shared_ptr<GameEvent> event)
{
	// Get player entity first
	auto entity = _entityMap->find(event->playerId)->second;

	// First remove from dogs or humans vector
	if (entity->getState()->type == ENTITY_DOG)
	{
		_gameState->dogs.erase(
			std::find(
				_gameState->dogs.begin(),
				_gameState->dogs.end(),
				entity->getState()->id));
	}
	else if (entity->getState()->type == ENTITY_HUMAN)
	{
		_gameState->humans.erase(
			std::find(
				_gameState->humans.begin(),
				_gameState->humans.end(),
				entity->getState()->id));
	}

	// Mark entity for deletion
	entity->getState()->isDestroyed = true;
	entity->hasChanged = true;
}
