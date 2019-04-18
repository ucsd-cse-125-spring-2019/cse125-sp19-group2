#include <ctime>
#include <time.h>
#include <random>
#include <glm/glm.hpp>

#include "Shared/QuadTree.hpp"
#include "SExampleEntity.hpp"
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

    // Start update loop
    this->update();
}


void GameServer::update()
{
    while (true)
    {
		// Random distribution
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 100);

		// TODO: create initial world, objects here
		// Random, test objects
		auto stateVec = std::vector<std::shared_ptr<BaseState>>();
		for (int i = 0; i < 100; i++)
		{
			auto state = std::make_shared<BaseState>();
			state->pos = glm::vec3(dis(gen), 0, dis(gen));
			state->width = 1;
			state->depth = 1;
			stateVec.push_back(state);
		}

        auto timerStart = std::chrono::steady_clock::now();

		BoundingBox box;
		box.pos = glm::vec2(50, 50);
		box.halfWidth = 50;
		QuadTree * tree = new QuadTree(box);

		for (auto& state : stateVec)
		{
			tree->insert(state.get());
		}

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

		std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
		exit(1);

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
