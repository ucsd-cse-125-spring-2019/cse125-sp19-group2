#pragma once

#include <vector>
#include <memory>
#include <winsock2.h>
#include <thread>
#include <chrono>
#include <iostream>

#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/BlockingQueue.hpp"

#define MAX_CONNECTIONS 10

class NetworkServer
{
private:
	/*
	** Runs in its own thread; created by by the constructor. Spawns a new
	** thread with socketHandler() for each new player connection.
	*/
	void connectionListener(uint8_t maxConnections);

	/*
	** Pulls from player socket and adds to the _eventQueue, sends to
	** player socket and removes from _updateQueue. One instance per player.
	*/
	void socketHandler(SOCKET socket, uint32_t playerId);

	// Queues for events and entity updates
	std::unique_ptr<std::queue<std::shared_ptr<GameEvent>>> _eventQueue;
	std::unique_ptr<BlockingQueue<std::shared_ptr<BaseState>>> _updateQueue;
	
	// thread that listens for connections
	std::thread _listener;
public:
	/*
	** Initialize queues, spawn a thread with connectionListener() to listen
	** for new player connections.
	**
	** TODO: write and pass in IDGenerator
	*/
	NetworkServer(std::string address, uint8_t port);

	/*
	** Close connections and destroy queues.
	*/
	~NetworkServer();

	/*
	** Returns the contents of the _eventQueue, removing them from the queue.
	*/
	std::vector<std::shared_ptr<GameEvent>> receiveEvents();

	/*
	** Add events to the _updateQueue(), to be sent by socketHandler()
	*/
	void sendUpdates(std::vector<std::shared_ptr<BaseState>>);
};

