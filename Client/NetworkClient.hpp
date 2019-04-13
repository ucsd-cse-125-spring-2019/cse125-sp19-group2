#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/BlockingQueue.hpp"

#define RECV_BUFSIZE 8192
#define SEND_BUFSIZE 8192

/*
** Class to interact with a server over the network. Public documentation
** marked with "API".
*/
class NetworkClient
{
public:
	/*
	** API: This will initialize the internal structures of the client. Be sure
    ** to call before connect().
    **
    ** Internal: Initialize queues.
	*/
	NetworkClient();

	/*
	** Internal: Destroy queues and any other allocated structures.
	*/
	~NetworkClient();

	/*
    ** API: Call with address and port to connect to server, returns a player
    ** ID. After this call, the closeConnection(), sendEvents(), and
    ** receiveUpdates() functions can be called on the client. Synchronous.
    **
	** Throws: std::runtime_error if connection to server fails
    **
	** Internal: Called from the main thread in client. Connects to server,
    ** creates a socket, spawns a thread with connect(), and returns a player
    ** ID.
	*/
	uint32_t connect(std::string address, std::string port);

	/*
    ** API: Call this function to disconnect from the server. After this call,
    ** the only valid function to call on the client is connect(). Synchronous.
    **
    ** Note that this function is called automatically if the server goes down
    ** or there are other socket errors. It is therefore imperative to use
    ** try-catch blocks on the sendEvents() and receiveUpdates() functions to
    ** ensure that the client smoothly rides through a disconnect.
    **
	** Internal: Shuts down internal I/O threads, clears queues, and closes
    ** the connected socket.
	*/
	void closeConnection();

	/*
    ** API: Send events to the server. Events will be queued internally and
    ** sent on a different thread. Synchronous for the calling thread;
    ** asynchronous with respect to the network.
    **
    ** Throws: std::runtime_error if not connected to server
    **
	** Internal: Add events to the _eventQueue, to be sent by socketHandler().
    **
	*/
	void sendEvents(std::vector<std::shared_ptr<GameEvent>> events);

	/*
    ** API: Receive updates from the server. Synchronous for the calling
    ** thread; asynchronous with respect to the network.
    **
    ** Throws: std::runtime_error if not connected to server
    **
	** Internal: Return contents of _updateQueue as a vector, removing them
    ** from the queue.
	*/
	std::vector<std::shared_ptr<BaseState>> receiveUpdates();

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

	// Status of the current connection, used to control I/O threads
	volatile bool _isAlive = true;

	// Lock for socket, used in closeConnection() and public-facing API funcs
	std::mutex _socketMutex;

	// I/O threads
	std::thread _readThread, _writeThread;
};

