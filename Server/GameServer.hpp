#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Shared/Logger.hpp"
#include "NetworkServer.hpp"
#include "SBaseEntity.hpp"

using tick = std::chrono::duration<double, std::ratio<1, TICKS_PER_SEC>>;

class GameServer
{
public:
	GameServer();
	~GameServer();

    // Setup components and start main loop
    void start();

    // Main update loop
    void update();

private:
    // Interface for client communication
    std::unique_ptr<NetworkServer> _networkInterface;

    // Map of all game entities
    std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>> _entityMap;

	// Resolve collisions
	void handleCollisions();

	// AABB collision resolution
	void handleAABB(BaseState* stateA, BaseState* stateB);

	// Capsule collision resolution
	void handleCapsule(BaseState* stateA, BaseState* stateB);
};

