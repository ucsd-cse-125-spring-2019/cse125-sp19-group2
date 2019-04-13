#pragma once

#include <vector>
#include <memory>
#include <iostream>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 8192

#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/BlockingQueue.hpp"

class NetworkClient
{
private:
	/*
	** Runs in its own thread; adds updates to _updateQueue from socket.
	*/
	void socketReadHandler();

	/*
	** Runs in its own thread; pulls events from _eventQueue and sends them
	** to the socket.
	*/
	void socketWriteHandler();
	
	// Blocking queue for events
	std::unique_ptr<BlockingQueue<std::shared_ptr<GameEvent>>> _eventQueue;

	// Queue and mutex for updates
	std::unique_ptr<std::queue<std::shared_ptr<BaseState>>> _updateQueue;
	std::mutex _updateMutex;

	// Socket to read and write from
	SOCKET _socket;

	// Status of the current connection
	volatile bool _isAlive = true;

	// Lock for socket, used only in closeConnection()
	std::mutex _socketMutex;

	// Threads for reading and writing
	std::thread _readThread, _writeThread;
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
	uint32_t connect(std::string address, std::string port);

	/*
	** Close active connection and cleanup.
	*/
	void closeConnection();

	/*
	** Add events to the _eventQueue, to be sent by socketHandler().
	*/
	void sendEvents(std::vector<std::shared_ptr<GameEvent>> events);

	/*
	** Return contents of _updateQueue as a vector, removing them from the
	** queue.
	*/
	std::vector<std::shared_ptr<BaseState>> receiveUpdates();
};

