#include <ctime>
#include <time.h>
#include <random>
#include <glm/glm.hpp>

#include "TextLevelParser.hpp"
#include "SDogEntity.hpp"
#include "SHumanEntity.hpp"
#include "SBoxEntity.hpp"
#include "GameServer.hpp"

// Allow pairs inside unordered_set
struct PairHash
{
	template <class T1, class T2>
	std::size_t operator () (std::pair<T1, T2> const &pair) const
	{
		std::size_t h1 = std::hash<T1>()(pair.first);
		std::size_t h2 = std::hash<T2>()(pair.second);

		return h1 ^ h2;
	}
};

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

    // Initialize object map
    _entityMap = std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>();

	// Init spawn point queues and jail location queues
	_jails = std::queue<glm::vec2>();
	_humanSpawns = std::queue<glm::vec2>();
	_dogSpawns = std::queue<glm::vec2>();

	// Init level parser and load level
	_levelParser = std::make_unique<TextLevelParser>();

	// Map initialization
	auto entities = _levelParser->parseLevelFromFile(
		"Levels/basic_map.txt",
		_jails,
		_humanSpawns,
		_dogSpawns);

	// Insert generated entities into global map
	for (auto& entity : entities)
	{
		_entityMap.insert({ entity->getState()->id, entity });
	}

    // Start update loop
    this->update();
}


void GameServer::update()
{
    while (true)
    {
        auto timerStart = std::chrono::steady_clock::now();
        /*** Begin Loop ***/

		// Get events from clients
		auto playerEvents = _networkInterface->receiveEvents();

        // Loop over player events
        for (auto& playerEvent : playerEvents)
        {
			// If the player just joined, create a player entity and send the
			// state of every object on the server
			if (playerEvent->type == EVENT_PLAYER_JOIN)
			{
				Logger::getInstance()->debug(
						std::string("\"") + playerEvent->playerName +
						std::string("\" joined the server!"));

				std::shared_ptr<SBaseEntity> playerEntity;

				// Make player entity; for now, even numbers are human, odd are dogs
				if (playerEvent->playerId % 2)
				{
					// Get first element from human spawn locations and push it back
					glm::vec2 humanSpawn = _humanSpawns.front();
					_humanSpawns.pop();
					_humanSpawns.push(humanSpawn);

					// Create player entity at position
					playerEntity = std::make_shared<SHumanEntity>(playerEvent->playerId);
					playerEntity->getState()->pos = glm::vec3(humanSpawn.x, 0, humanSpawn.y);
					_humans.push_back(playerEntity);
				}
				else
				{
					// Get first element from dog spawn locations and push it back
					glm::vec2 dogSpawn = _dogSpawns.front();
					_dogSpawns.pop();
					_dogSpawns.push(dogSpawn);

					// Create player entity at position
					playerEntity = std::make_shared<SDogEntity>(playerEvent->playerId);
					playerEntity->getState()->pos = glm::vec3(dogSpawn.x, 0, dogSpawn.y);
					_dogs.push_back(playerEntity);
				}

				// Throw it into server-wide map
				_entityMap.insert(std::pair<uint32_t,
						std::shared_ptr<SBaseEntity>>(
						playerEntity->getState()->id, playerEntity));

				// Generate a vector of all object states
				auto updateVec = std::vector<std::shared_ptr<BaseState>>();
				for (auto& entityPair : _entityMap)
				{
					updateVec.push_back(entityPair.second->getState());
				}

				// Send updates to this player only
				_networkInterface->sendUpdates(updateVec, playerEvent->playerId);

				// Also send this player entity to everyone. Results in a
				// duplicate update for the player who joined, but is cleaner
				// than a boolean check inside update()
				_networkInterface->sendUpdate(playerEntity->getState());

				// If we now have two players, start the game
				if (_humans.size() + _dogs.size() == 2)
				{
					_gameStarted = true;
					Logger::getInstance()->debug("Game Started!");
				}
			}

			// If a player has left, destroy their entity
			if (playerEvent->type == EVENT_PLAYER_LEAVE)
			{
				// Find player entity first
				auto it = _entityMap.find(playerEvent->playerId);
				if (it != _entityMap.end())
				{
					auto playerEntity = it->second;

					// Send an update with deleted flag set to true
					playerEntity->getState()->isDestroyed = true;
					_networkInterface->sendUpdate(playerEntity->getState());

					// Remove the entity from the map
					_entityMap.erase(it);
				}
				else
				{
					Logger::getInstance()->warn(
						std::string("Could not find player entity with ID: ") +
						std::to_string(playerEvent->playerId));
				}
			}
        }

        // General game logic

		// There are two identical for loops here; this is intentional, because
		// we need to update ALL objects on the server before sending updates
		for (auto& entityPair : _entityMap)
		{
			entityPair.second->update(playerEvents);
		}

		// Collision resolution
		handleCollisions();

		// Update state of the game
		bool dogsCaught = true;
		for (auto& dog : _dogs)
		{
			dogsCaught &= std::static_pointer_cast<SDogEntity>(dog)->isCaught;
		}

		if (_gameStarted && dogsCaught)
		{
			_gameStarted = false;
			_gameOver = true;
			Logger::getInstance()->debug("Humans won!");
		}
		else if (_gameStarted && _gameDuration > MAX_GAME_LENGTH)
		{
			_gameStarted = false;
			_gameOver = true;
			Logger::getInstance()->debug("Dogs won!");
		}

        // Send updates to clients
        auto updates = std::vector<std::shared_ptr<BaseState>>();
        for (auto& entityPair : _entityMap)
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

        /*** End Loop ***/

        auto timerEnd = std::chrono::steady_clock::now();

        // Elapsed time in nanoseconds
        auto elapsed = timerEnd - timerStart;

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

		// If game is running, increment timer
		if (_gameStarted)
		{
			_gameDuration += (std::chrono::steady_clock::now() - timerStart);
		}
    }
}

void GameServer::handleCollisions()
{
	// Build quadtree
	auto tree = new QuadTree({ glm::vec2(0), MAP_WIDTH / 2 });
	for (auto& entityPair : _entityMap)
	{
		tree->insert(entityPair.second->getState().get());
	}

	auto collisionSet = std::unordered_set<std::pair<BaseState*, BaseState*>, PairHash>();

	// Build set of pairs of collisions
	for (auto& entityPair : _entityMap)
	{
		auto entity = entityPair.second;

		// Only run collision check if not static
		if (!entity->getState()->isStatic)
		{
			for (auto& collidingEntity : entity->getColliding(*tree))
			{
				collisionSet.insert({ entity->getState().get(), collidingEntity });
			}
		}
	}

	// Iterate until no collisions
	while (!collisionSet.empty())
	{
		auto collisionPair = (collisionSet.begin());
		auto objectA = collisionPair->first;
		auto objectB = collisionPair->second;

		// Erase from beginning
		collisionSet.erase(collisionPair);

		if (objectA->type == ENTITY_DOG && objectB->type == ENTITY_HUMAN)
		{
			handleDogCaught(objectA, objectB, collisionSet, tree);
			continue;
		}

		if (objectA->type == ENTITY_HUMAN && objectB->type == ENTITY_DOG)
		{
			handleDogCaught(objectB, objectA, collisionSet, tree);
			continue;
		}

		// We know that A is a player (capsule collider), so switch on b only
		switch (objectB->colliderType)
		{
		case COLLIDER_CAPSULE:
		{
			handleCapsule(objectA, objectB);
			break;
		}

		case COLLIDER_AABB:
		{
			handleAABB(objectA, objectB);
			break;
		}

		} // switch

		// Remove duplicates (e.g. <A,B> vs <B,A> if both players)
		if (!objectB->isStatic)
		{
			collisionSet.erase({ objectB, objectA });

			// Mark as changed
			auto result = _entityMap.find(objectB->id);

			if (result != _entityMap.end())
			{
				result->second->hasChanged = true;
			}
		}

		// Find entity for object A
		auto result = _entityMap.find(objectA->id);

		if (result == _entityMap.end())
		{
			std::runtime_error("Could not find entity in handleCollisions()");
		}

		auto entity = result->second;
		entity->hasChanged = true;

		// Re-check for colliding
		for (auto& collidingEntity : entity->getColliding(*tree))
		{
			collisionSet.insert({ entity->getState().get(), collidingEntity });
		}
	}

	delete tree;
}


void GameServer::handleAABB(BaseState* stateA, BaseState* stateB)
{
	float rA = (float)std::fmax(stateA->width, stateA->depth) / 2;

	// Check which side of box the collision happens on
	float dists[4];
	dists[0] = (stateB->pos.x - stateB->width / 2) - stateA->pos.x; // West
	dists[1] = stateA->pos.x - (stateB->pos.x + stateB->width / 2); // East
	dists[2] = stateA->pos.z - (stateB->pos.z + stateB->depth / 2); // North
	dists[3] = (stateB->pos.z - stateB->depth / 2) - stateA->pos.z; // South

	// Check if collision happens at corner
	glm::vec3 cornerPos = stateB->pos;
	bool inCorner = false;
	if (dists[0] > 0) {
		cornerPos.x -= stateB->width / 2;
		if (dists[2] > 0) {
			cornerPos.z += stateB->depth / 2;
			inCorner = true;
		}
		else if (dists[3] > 0) {
			cornerPos.z -= stateB->depth / 2;
			inCorner = true;
		}
	} else if (dists[1] > 0) {
		cornerPos.x += stateB->width / 2;
		if (dists[2] > 0) {
			cornerPos.z += stateB->depth / 2;
			inCorner = true;
		}
		else if (dists[3] > 0) {
			cornerPos.z -= stateB->depth / 2;
			inCorner = true;
		}
	}

	// Collision happens at corner, 
	if (inCorner) {
		// Vector from corner to A
		glm::vec3 diff = stateA->pos - cornerPos;
		
		// We don't considerate height
		diff.y = 0;

		float overlap = rA - glm::length(diff);

		// Vector to move circles by
		glm::vec3 correctionVec;

		// Normal case: some displacement between circles
		if (glm::length(diff))
		{
			// How much the circles overlap, and the ratio of overlap to distance
			// between circles
			float ratio = overlap / glm::length(diff);
			correctionVec = diff * ratio;
		}
		// Edge case: objects directly on top each other
		else
		{
			correctionVec = glm::vec3(1, 0, 0) * overlap;
		}

		// Apply to A
		stateA->pos += correctionVec;
	}
	else {
		int minIndex = -1;
		float min = FLT_MAX;

		// Get closest edge
		for (int i = 0; i < 4; i++)
		{
			if (dists[i] > 0)
			{
				minIndex = i;
				break;
			}
			
			if (dists[i] < min)
			{
				min = dists[i];
				minIndex = i;
			}
		}

		glm::vec3 correctionVec = glm::vec3(0);
		switch (minIndex)
		{
		case 0: // West
			correctionVec.x = -(rA - dists[0]);
			break;
		case 1: // East
			correctionVec.x = rA - dists[1];
			break;
		case 2: // North
			correctionVec.z = rA - dists[2];
			break;
		case 3: // South
			correctionVec.z = -(rA - dists[3]);
			break;
		}

		stateA->pos += correctionVec;
	}
}


void GameServer::handleCapsule(BaseState* stateA, BaseState* stateB)
{
	float rA = (float)std::fmax(stateA->width, stateA->depth) / 2;
	float rB = (float)std::fmax(stateB->width, stateB->depth) / 2;

	// Vector from B to A
	glm::vec3 diff = stateA->pos - stateB->pos;

	float overlap = rA + rB - glm::length(diff);

	// Vector to move circles by
	glm::vec3 correctionVec;

	// Normal case: some displacement between circles
	if (glm::length(diff))
	{
		// How much the circles overlap, and the ratio of overlap to distance
		// between circles
		float ratio = overlap / glm::length(diff);
		correctionVec = diff * ratio;
	}
	// Edge case: objects directly on top each other
	else
	{
		correctionVec = glm::vec3(1, 0, 0) * overlap;
	}

	// If two movable objects, only move halfway
	if (!stateB->isStatic)
	{
		correctionVec /= 2.0f;

		// Apply to B
		stateB->pos -= correctionVec;
	}

	// Apply to A
	stateA->pos += correctionVec;
}

void GameServer::handleDogCaught(
	BaseState* dog,
	BaseState* human,
	std::unordered_set<std::pair<BaseState*, BaseState*>, PairHash> & collisionSet,
	QuadTree * tree)
{
	// Find entity for dog
	auto result = _entityMap.find(dog->id);

	if (result == _entityMap.end())
	{
		std::runtime_error("Could not find entity in handleCollisions()");
	}

	auto entity = result->second;
	entity->hasChanged = true;

	// Cast as dog entity and mark as caught
	auto dogEntity = std::static_pointer_cast<SDogEntity>(entity);
	dogEntity->isCaught = true;

	// Removing all colliding with dog
	for (auto& collidingEntity : entity->getColliding(*tree))
	{
		collisionSet.erase({ entity->getState().get(), collidingEntity });
		collisionSet.erase({ collidingEntity, entity->getState().get() });
	}

	// Get jail location from queue
	glm::vec2 jailPos = _jails.front();
	_jails.pop();
	_jails.push(jailPos);

	dog->pos = glm::vec3(jailPos.x, 0, jailPos.y);
}