#pragma once

#include <vector>
#include <memory>
#include <winsock2.h>

#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/BlockingQueue.hpp"

class NetworkClient
{
private:
	/*
	** Runs in its own thread; created by connect(). Pulls from server socket
	** and adds to the _updateQueue, sends to server socket and removes
	** from the _eventQueue.
	*/
	void socketHandler(SOCKET socket);
	
	// Queues for events and entity updates
	std::unique_ptr<std::shared_ptr<BlockingQueue<GameEvent>>> _eventQueue;
	std::unique_ptr<std::shared_ptr<std::queue<BaseState>>> _updateQueue;

public:
	/*
	** Initialize queues.
	*/
	NetworkClient();

	/*
	** Destroy queues and any other allocated structures.
	*/
	~NetworkClient();

	/*
	** Called from the main thread in client. Connects to server, creates a
	** socket, spawns a thread with connect(), and returns a player ID.
	**
	** TODO: exceptions
	*/
	uint32_t connect(std::string address, uint8_t port);

	/*
	** Add events to the _eventQueue, to be sent by socketHandler().
	*/
	void sendEvents(std::vector<std::shared_ptr<GameEvent>>);

	/*
	** Return contents of _updateQueue as a vector, removing them from the
	** queue.
	*/
	std::vector<std::shared_ptr<BaseState>> receiveUpdates();
};

