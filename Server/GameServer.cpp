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
	// Seed RNG with current time
	std::srand(unsigned(std::time(0)));

    // Start network server
    _networkInterface = std::make_unique<NetworkServer>(PORTNUM);

    // Initialize object map
    _entityMap = std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>();

	// Init spawn point queues and jail location queues
	_jails = std::vector<glm::vec2>();
	_humanSpawns = std::queue<glm::vec2>();
	_dogSpawns = std::queue<glm::vec2>();

	// Init level parser and load level
	_levelParser = std::make_unique<GridLevelParser>();

	// Init collision manager
	_collisionManager = std::make_unique<CollisionManager>(&_entityMap, &_jails);

	// Map initialization
	auto entities = _levelParser->parseLevelFromFile(
		"Levels/map.dat",
		_jails,
		_humanSpawns,
		_dogSpawns);

	Logger::getInstance()->debug("Parsed " + std::to_string(entities.size()) + " entities from file.");

	// Ensure at least one human spawn, dog spawn, and jail
	// TODO: convert these to exceptions, or find a cleaner way to handle this
	if (!_jails.size())
	{
		Logger::getInstance()->fatal("No jails found in level file");
		fgetc(stdin);
	}
	if (!_humanSpawns.size())
	{
		Logger::getInstance()->fatal("No human spawn locations found in level file");
		fgetc(stdin);
	}
	if (!_dogSpawns.size())
	{
		Logger::getInstance()->fatal("No dog spawn locations found in level file");
		fgetc(stdin);
	}

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
					playerEntity = std::make_shared<SDogEntity>(playerEvent->playerId, &_jails);
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
		_collisionManager->handleCollisions();

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
