#include <ctime>
#include <time.h>
#include <random>
#include <glm/glm.hpp>

#include "Shared/QuadTree.hpp"
#include "SPlayerEntity.hpp"
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

	// TODO: create initial world, objects here


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

				// Make player entity
				std::shared_ptr<SBaseEntity> playerEntity =
						std::make_shared<SPlayerEntity>(playerEvent->playerId);

				// Throw it into server-wide map
				_entityMap.insert(std::pair<uint32_t,
						std::shared_ptr<SBaseEntity>>(
						playerEntity->getState(true)->id, playerEntity));

				// Generate a vector of all object states
				auto updateVec = std::vector<std::shared_ptr<BaseState>>();
				for (auto& entityPair : _entityMap)
				{
					updateVec.push_back(entityPair.second->getState(true));
				}

				// Send updates to this player only
				_networkInterface->sendUpdates(updateVec, playerEvent->playerId);

				// Also send this player entity to everyone. Results in a
				// duplicate update for the player who joined, but is cleaner
				// than a boolean check inside update()
				_networkInterface->sendUpdate(playerEntity->getState(true));
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
					playerEntity->getState(true)->isDestroyed = true;
					_networkInterface->sendUpdate(playerEntity->getState(true));

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

        // Send updates to clients
        auto updates = std::vector<std::shared_ptr<BaseState>>();
        for (auto& entityPair : _entityMap)
        {
            uint32_t id = entityPair.first;
            std::shared_ptr<SBaseEntity> entity = entityPair.second;

            // Add to vector only if there is an update available
			if (entity->getState())
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

		//Logger::getInstance()->debug("Elapsed time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()));

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

void GameServer::handleCollisions()
{
	// Build quadtree
	auto tree = new QuadTree({ glm::vec2(0), MAP_WIDTH / 2 });
	for (auto& entityPair : _entityMap)
	{
		tree->insert(entityPair.second->getState(true).get());
	}

	auto collisionSet = std::unordered_set<std::pair<BaseState*, BaseState*>, PairHash>();

	// Build set of pairs of collisions
	for (auto& entityPair : _entityMap)
	{
		auto entity = entityPair.second;

		// Only run collision check if not static
		if (!entity->getState(true)->isStatic)
		{
			for (auto& collidingEntity : entity->getColliding(*tree))
			{
				collisionSet.insert({ entity->getState(true).get(), collidingEntity });
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
			collisionSet.insert({ entity->getState(true).get(), collidingEntity });
		}
	}

	delete tree;
}


void GameServer::handleAABB(BaseState* stateA, BaseState* stateB)
{
	// TODO
}


void GameServer::handleCapsule(BaseState* stateA, BaseState* stateB)
{
	float rA = (float)std::fmax(stateA->width, stateA->depth) / 2;
	float rB = (float)std::fmax(stateB->width, stateB->depth) / 2;

	// Vector from B to A
	glm::vec3 diff = stateA->pos - stateB->pos;

	// How much the circles overlap, and the ratio of overlap to distance
	// between circles
	float overlap = rA + rB - glm::length(diff);
	float ratio = overlap / glm::length(diff);

	glm::vec3 correctionVec = diff * ratio;

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

