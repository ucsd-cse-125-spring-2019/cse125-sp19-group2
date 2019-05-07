#pragma once

#include <vector>
#include <memory>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <shared_mutex>
#include <utility>

#include "Shared/BaseState.hpp"
#include "Shared/GameEvent.hpp"
#include "Shared/BlockingQueue.hpp"
#include "IdGenerator.hpp"

#define MAX_CONNECTIONS 10
#define SELECT_TIMEOUT_SEC 1   // Timeout for the select() syscall
#define RECV_BUFSIZE 8192
#define SEND_BUFSIZE 8192

/*
** Class to interact with clients over the network. Public documentation marked
** with "API".
**
** Note that there is currently no conversion of endianness to network byte
** order (and vice versa), because it would be more overhead for little gain.
** x86 is little endian, and we will not be working with big endian machines.
**
** If this changes in the future, we should instead use the functionality in
** <cereal/archives/binary_portable.hpp>, and tweak the way we send uint32_t's.
*/
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
	** API: Forcefully disconnects a player from the server. Synchronous.
	**
	** Throws: runtime_exception if the playerId could not be found.
	**
	** Internal: Removes player session with playerId from _sessions.
	*/
	void closePlayerSession(uint32_t playerId);

	/*
	** API: receive events from all clients. No guarantees on client order, but
	** events should be received in order on a per-client basis. Synchronous
	** for the calling thread; asynchronous with respect to the network. Note
	** that player ID's inside the GameEvent struct are overwritten based on
	** the player's socket, so it is safe to send any kind of event from the
	** client side.
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
	** Internal: Add updates to the _updateQueue, to be sent by
	** socketHandler(). PlayerID is set to 0 in the pairs added to the queue.
	*/
	void sendUpdates(std::vector<std::shared_ptr<BaseState>> updates);

	/*
	** Same as above, except it sends a single update only.
	*/
	void sendUpdate(std::shared_ptr<BaseState> update);

	/*
	** API: Send updates to a particular client. Use this if updates need to be
	** sent only to a certain player (at login, for example). See the function
	** above for other details. Note that if there is no player with the
	** specified ID, the updates will be lost.
	**
	** Internal: Works the same way as above, except PlayerIDs in the pairs are
	** set to the value that is passed to this function.
	**/
	void sendUpdates(std::vector<std::shared_ptr<BaseState>> updates, uint32_t playerId);

	/*
	** Same as above, except it sends a single update only.
	*/
	void sendUpdate(std::shared_ptr<BaseState> update, uint32_t playerId);


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

	// Event queue and mutex
	std::unique_ptr<std::queue<std::shared_ptr<GameEvent>>> _eventQueue;
	std::mutex _eventMutex;

	// Blocking update queue. Stores pairs of playerIDs and BaseState pointers.
	// If the playerID is zero, the update is sent to all clients; otherwise,
	// it is sent to the socket mapped to that playerID.
	std::unique_ptr<BlockingQueue<std::pair<uint32_t, std::shared_ptr<BaseState>>>> _updateQueue;
	
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
		char* readBuf;
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

