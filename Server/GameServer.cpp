#include <ctime>
#include <time.h>

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
    int counter = 0;
    while (true)
    {
        auto timerStart = std::chrono::steady_clock::now();



        /*** Begin Loop ***/

        // Loop over player events
        for (auto& playerEvent : _networkInterface->receiveEvents())
        {
            // TODO
        }

        // TODO: game logic and physics simulation (if any) go here.
  


        // Send updates to clients. To be optimized
        auto updates = std::vector<std::shared_ptr<BaseState>>();
        for (auto& entityPair : _entityMap)
        {
            uint32_t id = entityPair.first;
            std::shared_ptr<SBaseEntity> entity = entityPair.second;

            // TODO: check to see if we should send this update first
            updates.push_back(entity->getState());
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
