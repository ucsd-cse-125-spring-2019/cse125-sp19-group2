#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <algorithm>

#include "Shared/Logger.hpp"
#include "NetworkServer.hpp"

NetworkServer::NetworkServer(std::string port)
{
	// Initialize queues
	_updateQueue = std::make_unique<BlockingQueue<std::pair<uint32_t, std::shared_ptr<BaseState>>>>();
	_eventQueue = std::make_unique<std::queue<std::shared_ptr<GameEvent>>>();

	_sessions = std::unordered_map<uint32_t, SocketState>();

	// Start listener thread
	_listenerThread = std::thread(
			&NetworkServer::connectionListener,
			this,
			port,
			MAX_CONNECTIONS);

    // Start socket read thread
    _readThread = std::thread(
            &NetworkServer::socketReadHandler,
            this);

    // Start socket write thread
    _writeThread = std::thread(
            &NetworkServer::socketWriteHandler,
            this);
}


NetworkServer::~NetworkServer()
{
	// TODO: destroy the queues
}


void NetworkServer::connectionListener(
		std::string port,
		uint8_t maxConnections)
{
	WSADATA wsaData;

	// Listen socket for new connections, temp socket for new clients
	SOCKET listenSock, tempSock;
	listenSock = tempSock = INVALID_SOCKET;

	// Address information
	struct addrinfo * result = NULL;
	struct addrinfo hints;

	// Init winsock
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res)
	{
        Logger::getInstance()->fatal("WSAStartup failed with error: " +
                std::to_string(res));
        fgetc(stdin);
		exit(1);
	}

	// Fill addr info struct
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Get address info from hints
	res = getaddrinfo(NULL, port.c_str(), &hints, &result);
	if (res)
	{
        Logger::getInstance()->fatal("getaddrinfo failed with error: " +
                std::to_string(res));
		WSACleanup();
        fgetc(stdin);
		exit(1);
	}

    // Create connection listener socket
	listenSock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (listenSock == INVALID_SOCKET)
	{
        Logger::getInstance()->fatal("Failed to create socket with error: " +
                std::to_string(WSAGetLastError()));
		freeaddrinfo(result);
		WSACleanup();
        fgetc(stdin);
		exit(1);
	}

    // Bind to address and port
	res = bind(listenSock, result->ai_addr, (int)result->ai_addrlen);

	if (res == SOCKET_ERROR)
	{
        Logger::getInstance()->fatal("Failed to bind socket with error: " +
                std::to_string(WSAGetLastError()));
		freeaddrinfo(result);
		closesocket(listenSock);
		WSACleanup();
        fgetc(stdin);
		exit(1);
	}

    // Put the socket in listening mode
	res = listen(listenSock, SOMAXCONN);

	if (res == SOCKET_ERROR)
	{
        Logger::getInstance()->fatal("Failed to put socket in listening mode " +
                std::string("with error: ") + std::to_string(WSAGetLastError()));
		closesocket(listenSock);
		WSACleanup();
        fgetc(stdin);
		exit(1);
	}

    Logger::getInstance()->info("Server listening for incoming connections on port " + port);

    // While the server is running, we want to be accepting new connections.
    // There is currently no cleanup logic, but it might be nice to have at
    // some point.
	while (true)
	{
        // This blocks until we get an incoming connection
		tempSock = accept(listenSock, NULL, NULL);

        // Reject connection if we are already at maxConnections.
		std::unique_lock<std::shared_mutex> lock(_sessionMutex);
		if (_sessions.size() >= maxConnections)
		{
            Logger::getInstance()->info("Rejecting new connection, server is full");
			closesocket(tempSock);
			continue;
		}
		lock.unlock();

        // Otherwise create a player session for the new socket
		if (tempSock != INVALID_SOCKET)
		{
			char value = 1;
			setsockopt(tempSock, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
			SocketState clientState =
			{
				IdGenerator::getInstance()->getNextId(), // create a new player id
				tempSock, // socket
				(char*)calloc(1, RECV_BUFSIZE), // read buffer
				false, // is reading
				0, // length (bytes to read)
				0, // bytes read
				std::vector<char>() // write buffer
			};

            Logger::getInstance()->info("Accepting new connection with playerId: " +
                    std::to_string(clientState.playerId));

			// Send player ID (4 bytes) at the beginning of connection
			int bytesSent = 0;
			while (bytesSent != sizeof(uint32_t))
			{
				int sendResult = send(
						tempSock,
						(char*)(&(clientState.playerId)),
						sizeof(uint32_t),
						0);

				if (sendResult > 0)
				{
					bytesSent += sendResult;
				}
				else if (!sendResult || sendResult == SOCKET_ERROR)
				{
                    Logger::getInstance()->error("Failed to send player ID to new client");
					free(clientState.readBuf);
					closesocket(tempSock);
					tempSock = INVALID_SOCKET;
					continue;
				}
			}

			// Set socket as non-blocking
			u_long socketMode = 1;
			res = ioctlsocket(tempSock, FIONBIO, &socketMode);

			// Check if error setting as non-blocking
			if (res == SOCKET_ERROR)
			{
                Logger::getInstance()->fatal(
                        "Failed to set socket as non-blocking. Error code: " +
                        std::to_string(res));
				free(clientState.readBuf);
				closesocket(tempSock);
				WSACleanup();
				exit(1);
			}

			// unlocks when out of scope
			std::unique_lock<std::shared_mutex> lock(_sessionMutex);

			// Add to session map
			_sessions.insert({clientState.playerId, clientState});

			tempSock = INVALID_SOCKET;
		}
		else
		{
            Logger::getInstance()->error(
                    "Client connection failed with error: " +
                    std::to_string(WSAGetLastError()));
		}
	}
}


void NetworkServer::socketReadHandler()
{
	// Read socket set for select()
	FD_SET readSet;
    FD_SET exceptSet;

    // String stream for reading from sockets
    std::stringstream ss;

    // List of dead sockets to kill
    std::queue<uint32_t> sessionsToKill = std::queue<uint32_t>();

    // Timeval struct for timeout
    TIMEVAL * timeout = (TIMEVAL *)calloc(1, sizeof(timeout));
    timeout->tv_sec = SELECT_TIMEOUT_SEC;

	while (true)
	{
		// Reset readSet and add sockets from session map
		FD_ZERO(&readSet);
        FD_ZERO(&exceptSet);
		std::shared_lock<std::shared_mutex> lock(_sessionMutex);

        // If no active sessions, sleep and restart the loop
        if (!_sessions.size())
        {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(SELECT_TIMEOUT_SEC));
            continue;
        }

        // Set read and except FDs
		for (auto& pair : _sessions)
		{
			FD_SET(pair.second.socket, &readSet);
            FD_SET(pair.second.socket, &exceptSet);
		}

		// Unlock and wait for read activity
		lock.unlock();

        if (select(0, &readSet, NULL, &exceptSet, timeout) == SOCKET_ERROR)
        {
            Logger::getInstance()->warn(
                    "select() returned error in read thread: " +
                    std::to_string(WSAGetLastError()));
        }
        else
        {
            std::shared_lock<std::shared_mutex> lock(_sessionMutex);

            // Iterate over sessions
            for (auto& pair : _sessions)
            {
                SocketState * session = &(pair.second);

                // If in the read set, recv and push to buffer
                if (FD_ISSET(session->socket, &readSet))
                {
                    int recvResult = recv(
                            session->socket,
                            session->readBuf + session->bytesRead,
                            RECV_BUFSIZE - session->bytesRead,
                            0);

                    // If zero, we had a clean disconnect, so mark as dead
                    if (!recvResult)
                    {
                        // Mark socket as dead
                        sessionsToKill.push(session->playerId);
                    }
                    else if (recvResult != SOCKET_ERROR)
                    {
                        session->bytesRead += recvResult;

						while ((session->isReading && (session->length + sizeof(uint32_t)) <= session->bytesRead) ||
								(!session->isReading && session->bytesRead >= sizeof(uint32_t)))
						{
							// If we're not in reading mode and received at least
							// four bytes, store length and go into reading mode.
							if (!session->isReading && session->bytesRead >= sizeof(uint32_t))
							{
								// Get the first four bytes and store as length
								memcpy(&(session->length), session->readBuf, sizeof(uint32_t));
								session->isReading = true;
							}

							// Deserialize object
							if (session->isReading && session->bytesRead >= (session->length + sizeof(uint32_t)))
							{
								ss.write(session->readBuf + sizeof(uint32_t), session->length);
								cereal::BinaryInputArchive iarchive(ss);
								std::shared_ptr<GameEvent> eventPtr;
								iarchive(eventPtr);

								// Enforce correct player ID
								eventPtr->playerId = session->playerId;

								// Lock and add to queue
								std::unique_lock<std::mutex> eventLock(_eventMutex);
								_eventQueue->push(eventPtr);
								eventLock.unlock();

								// reset state
								session->bytesRead -= (session->length + sizeof(uint32_t));
								session->isReading = false;

								// Effectively "remove" length and item from buffer
								memmove(
										session->readBuf,
										session->readBuf + session->length + sizeof(uint32_t), 
										RECV_BUFSIZE - (session->length + sizeof(uint32_t)));
							}
						}
                    }
                    else
                    {
                        // Otherwise we had a miscellaneous error. Mark as dead.
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {
                            Logger::getInstance()->info(
                                    "Encountered error while reading socket for player " +
                                    std::to_string(session->playerId) + " with code " +
                                    std::to_string(WSAGetLastError()));

                            sessionsToKill.push(session->playerId);
                        }
                    }
                }

                // If in the exception set, mark session as dead.
                if (FD_ISSET(session->socket, &exceptSet))
                {
                    // Mark socket as dead 
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        Logger::getInstance()->info(
                                "Encountered error while reading socket for player " +
                                std::to_string(session->playerId) + " with code " +
                                std::to_string(WSAGetLastError()));

                        sessionsToKill.push(session->playerId);
                    }
                }
            }

            // Kill sessions marked for death
            lock.unlock();
            while (!sessionsToKill.empty())
            {
                closePlayerSession(sessionsToKill.front());
                sessionsToKill.pop();
            }
        }
	}
}


void NetworkServer::socketWriteHandler()
{

    // List of dead sockets to kill
    std::queue<uint32_t> sessionsToKill = std::queue<uint32_t>();

    while (true)
    {
		// If no clients, sleep and restart the loop
		std::shared_lock<std::shared_mutex> preLock(_sessionMutex);
		if (!_sessions.size())
		{
			preLock.unlock();
			std::this_thread::sleep_for(std::chrono::seconds(SELECT_TIMEOUT_SEC));
			continue;
		}
        preLock.unlock();

        // Retreive next item from queue. This will block until an item appears
		// on the queue.
		std::pair<uint32_t, std::shared_ptr<BaseState>> nextPair;
		_updateQueue->pop(nextPair);

        // Grab player ID and shared pointer from pair
        uint32_t playerId = nextPair.first;
        std::shared_ptr<BaseState> nextItem = nextPair.second;

		// Create an output archive and serialize the object from the queue
		std::stringstream ss;
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(nextItem);

		// Get size of serialized object
        ss.seekg(0, std::ios::end);
        uint32_t size = (uint32_t)ss.tellg();
        ss.seekg(0, std::ios::beg);

		// Copy into char buffer
        char * databuf = (char*)malloc(size + sizeof(uint32_t));

		// Size of serialized object first, then object itself
		memcpy(databuf, &size, sizeof(uint32_t));
        ss.read(databuf + sizeof(uint32_t), size);

		// Lock and iterate over player sessions
        std::shared_lock<std::shared_mutex> lock(_sessionMutex);
        for (auto& pair : _sessions)
        {
            SocketState session = pair.second;

            // Only send the data if this is the correct playerId or if no
            // playerId was specified
            if (!playerId || session.playerId == playerId)
            {
                std::copy(
                    databuf,
                    databuf + size + sizeof(uint32_t),
                    std::back_inserter(session.writeBuf));

                // Loop until write buffer is empty
                while (session.writeBuf.size())
                {
                    int sendResult = send(
                        session.socket,
                        session.writeBuf.data(),
                        (int)session.writeBuf.size(),
                        0);

                    // If no error, shrink buffer
                    if (sendResult > 0)
                    {
                        session.writeBuf.erase(
                            session.writeBuf.begin(),
                            session.writeBuf.begin() + sendResult);
                    }
                    else if (sendResult == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        // Debug
                        Logger::getInstance()->info(
                                "Encountered error while writing socket for player " +
                                std::to_string(session.playerId) + " with code " +
                                std::to_string(WSAGetLastError()));

						sessionsToKill.push(session.playerId);
						break;
                    }
                }
            }
        }
        free(databuf);

		// Kill sessions marked for death
		lock.unlock();
		while (!sessionsToKill.empty())
		{
			closePlayerSession(sessionsToKill.front());
			sessionsToKill.pop();
		}
    }
    return;
}


std::vector<uint32_t> NetworkServer::getPlayerList()
{
    auto list = std::vector<uint32_t>();

    // Acquire read lock on session map and build list
    std::shared_lock<std::shared_mutex> lock(_sessionMutex);
    for (auto& session : _sessions)
    {
        list.push_back(session.first);
    }

    return list;
}


void NetworkServer::closePlayerSession(uint32_t playerId)
{
    std::unique_lock<std::shared_mutex> lock(_sessionMutex);

    // Get a copy of the session that we're going to close
    auto result = _sessions.find(playerId);

    // If found, close the socket and remove from the map.
    if (result != _sessions.end())
    {
        Logger::getInstance()->info(
                "Closing session for player " +
                std::to_string(playerId));

		free(result->second.readBuf);
        closesocket(result->second.socket);
        _sessions.erase(result);

		// Create a PLAYER_LEAVE event for the person that was DC'd
		auto leaveEvent = std::make_shared<GameEvent>();
		leaveEvent->playerId = playerId;
		leaveEvent->type = EVENT_PLAYER_LEAVE;

		// Lock and push to event queue
		std::unique_lock<std::mutex> eventLock(_eventMutex);
		_eventQueue->push(leaveEvent);
    }
    else
    {
        Logger::getInstance()->info(
                "Failed to find session for player " +
                std::to_string(playerId));

        // If this isn't the read thread, throw an exception
        if (std::this_thread::get_id() != _readThread.get_id())
        {
            throw(std::runtime_error("Failed to find session for player " + playerId));
        }
    }
}


std::vector<std::shared_ptr<GameEvent>> NetworkServer::receiveEvents()
{
	auto eventList = std::vector<std::shared_ptr<GameEvent>>();

	// Lock and build events from queue
	std::unique_lock<std::mutex> lock(_eventMutex);
	while (!_eventQueue->empty())
	{
		eventList.push_back(_eventQueue->front());
		_eventQueue->pop();
	}

	return eventList;
}


void NetworkServer::sendUpdates(std::vector<std::shared_ptr<BaseState>> updates)
{
    for (auto& update : updates)
    {
		sendUpdate(update);
    }
}


void NetworkServer::sendUpdate(std::shared_ptr<BaseState> update)
{
	// We are sending to all players, so use 0 as playerId
	std::pair<uint32_t, std::shared_ptr<BaseState>> updatePair = std::make_pair(0, update);
	_updateQueue->push(updatePair);
}


void NetworkServer::sendUpdates(std::vector<std::shared_ptr<BaseState>> updates, uint32_t playerId)
{
    for (auto& update : updates)
    {
		sendUpdate(update, playerId);
    }
}


void NetworkServer::sendUpdate(std::shared_ptr<BaseState> update, uint32_t playerId)
{
	// We are sending to just one player
	std::pair<uint32_t, std::shared_ptr<BaseState>> updatePair = std::make_pair(playerId, update);
	_updateQueue->push(updatePair);
}
