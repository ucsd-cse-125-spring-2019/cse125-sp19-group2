#pragma once

#include <vector>
#include <memory>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <shared_mutex>


#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/BlockingQueue.hpp"
#include "IdGenerator.hpp"

#define MAX_CONNECTIONS 2
#define SELECT_TIMEOUT_SEC 1
#define RECV_BUFSIZE 8192
#define SEND_BUFSIZE 8192

class NetworkServer
{
private:
	/*
	** Runs in its own thread; created by by the constructor. Accepts new
    ** connections and creates player sessions in _sessions.
	*/
	void connectionListener(
			std::string port,
			uint8_t maxConnections);

	/*
	** Pulls from player sockets and adds to the _eventQueue. Handles all
    ** active player connections. Inactive sockets are handled in this thread.
	*/
	void socketReadHandler();

	/*
	** Sends to player sockets and removes from the _updateQueue. Handles all
    ** active player connections. Does NOT handle dead connections.
	*/
    void socketWriteHandler();

    /*
    ** Removes player session with playerId from _sessions.
    */
    void closePlayerSession(uint32_t playerId);

	// Event queue and mutex
	std::unique_ptr<std::queue<std::shared_ptr<GameEvent>>> _eventQueue;
    std::mutex _eventMutex;

    // Blocking update queue
	std::unique_ptr<BlockingQueue<std::shared_ptr<BaseState>>> _updateQueue;
	
	// Network threads
	std::thread _listener, _readThread, _writeThread;

	struct SocketState
	{
		uint32_t playerId;
		SOCKET socket;

        // Read stuff
		std::vector<char> readBuf;
		bool isReading;
		uint32_t length;
		uint32_t bytesRead;

        // Write stuff
		std::vector<char> writeBuf;
	};

	// client session state map from player id to session
	std::unordered_map<uint32_t, SocketState> _sessions;
	mutable std::shared_mutex _sessionMutex;

public:
	/*
	** Initialize queues, spawn a thread with connectionListener() to listen
	** for new player connections.
	**
	** TODO: write and pass in IDGenerator
	*/
	NetworkServer(std::string port);

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

