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

#define MAX_CONNECTIONS 10
#define SELECT_TIMEOUT_SEC 1
#define RECV_BUFSIZE 8192
#define SEND_BUFSIZE 8192

class NetworkServer
{
public:
	/*
    ** API: This will initialize the network server and start listening for new
    ** connections.
    **
	** Internal: Initialize queues, spawn a thread with connectionListener()
    ** to listen for new player connections.
	*/
	NetworkServer(std::string port);

	/*
	** Internal: Close connections and destroy queues.
	*/
	~NetworkServer();

    /*
    ** API: Returns a vector of active player IDs. Synchronous.
    */
    std::vector<uint32_t> getPlayerList();

	/*
    ** API: receive events from all clients. No guarantees on client order, but
    ** events should be received in order on a per-client basis. Synchronous
    ** for the calling thread; asynchronous with respect to the network.
    **
    ** Internal: Returns the contents of the _eventQueue, removing them from
    ** the queue.
	*/
	std::vector<std::shared_ptr<GameEvent>> receiveEvents();

	/*
    ** API: Send updates to all clients. No guarantees on client order, but
    ** updates will be sent in order on a per-client basis. Try to avoid
    ** calling this function when no clients are connected, as updates will
    ** fill an internal queue. Synchronous for the calling thread;
    ** asynchronous with respect to the network.
    **
	** Internal: Add events to the _updateQueue(), to be sent by
    ** socketHandler().
	*/
	void sendUpdates(std::vector<std::shared_ptr<BaseState>>);

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
	
	// I/O threads
	std::thread _listenerThread, _readThread, _writeThread;

    /*
    ** Basic private struct to keep track of internal state on a per-client
    ** basis.
    */
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

	// Client session state map from player id to session
	std::unordered_map<uint32_t, SocketState> _sessions;
	mutable std::shared_mutex _sessionMutex;
};

