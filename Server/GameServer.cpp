#include <ctime>
#include <time.h>
#include <random>
#include <glm/glm.hpp>

#include "Shared/QuadTree.hpp"
#include "SExampleEntity.hpp"
#include "SPlayerEntity.hpp"
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

        // Loop over player events
        for (auto& playerEvent : _networkInterface->receiveEvents())
        {
			// If the player just joined, create a player entity and send the
			// state of every object on the server
			if (playerEvent->type == EVENT_PLAYER_JOIN)
			{
				Logger::getInstance()->debug(
						std::string("\"") + playerEvent->playerName +
						std::string("\" joined the server!"));

				// Make player entity
				auto playerEntity = std::make_shared<SPlayerEntity>(playerEvent->playerId);

				// Throw it into server-wide map
				_entityMap.insert(std::pair<uint32_t,
						std::shared_ptr<SBaseEntity>>(playerEntity->getState()->id, playerEntity));

				// Generate a vector of all object states
				auto updateVec = std::vector<std::shared_ptr<BaseState>>();
				for (auto& entityPair : _entityMap)
				{
					updateVec.push_back(entityPair.second->getState(true));
				}

				// Send updates to this player only
				_networkInterface->sendUpdates(updateVec, playerEvent->playerId);
			}

			// If a player has left, destroy their entity
			if (playerEvent->type == EVENT_PLAYER_LEAVE)
			{
				// TODO
			}
        }

        // TODO: game logic and physics simulation (if any) go here.
  


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

        /*** End Loop ***/



        auto timerEnd = std::chrono::steady_clock::now();

        // Elapsed time in nanoseconds
        auto elapsed = timerEnd - timerStart;

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
