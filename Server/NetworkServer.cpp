#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <algorithm>

#include "NetworkServer.hpp"

NetworkServer::NetworkServer(std::string port)
{
	// Initialize queues
	_updateQueue = std::make_unique<BlockingQueue<std::shared_ptr<BaseState>>>();
	_eventQueue = std::make_unique<std::queue<std::shared_ptr<GameEvent>>>();

	_sessions = std::unordered_map<uint32_t, SocketState>();

	// Start listener thread
	_listener = std::thread(
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
		std::cerr << "WSAStartup failed with error: " << res << std::endl;
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
		std::cerr << "getaddrinfo failed with error: " << res << std::endl;
		WSACleanup();
		exit(1);
	}

	listenSock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (listenSock == INVALID_SOCKET)
	{
		std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	res = bind(listenSock, result->ai_addr, (int)result->ai_addrlen);

	if (res == SOCKET_ERROR)
	{
		std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(listenSock);
		WSACleanup();
		exit(1);
	}

	res = listen(listenSock, SOMAXCONN);

	if (res == SOCKET_ERROR)
	{
		std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
		closesocket(listenSock);
		WSACleanup();
		exit(1);
	}

	while (true)
	{
		tempSock = accept(listenSock, NULL, NULL);

		std::unique_lock<std::shared_mutex> lock(_sessionMutex);
		if (_sessions.size() >= maxConnections)
		{
			closesocket(tempSock);
			continue;
		}
		lock.unlock();
		if (tempSock != INVALID_SOCKET)
		{
			char value = 1;
			setsockopt(tempSock, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
			SocketState clientState =
			{
				IdGenerator::getInstance()->getNextId(), // create a new player id
				tempSock, // socket
				std::vector<char>(RECV_BUFSIZE), // read buffer
				false, // is reading
				0, // length (bytes to read)
				0, // bytes read
				std::vector<char>(SEND_BUFSIZE) // write buffer
			};

			std::cerr << "Accepting new connection with playerId: "
					<< clientState.playerId << std::endl;


			// Send player ID (4 bytes) at the beginning of connection
			if (send(tempSock,
					(char*)(&(clientState.playerId)), 
					sizeof(uint32_t), 0) == sizeof(uint32_t))
			{
				// Set socket as non-blocking
				u_long socketMode = 1;
				res = ioctlsocket(tempSock, FIONBIO, &socketMode);

				// Check if error setting as non-blocking
				if (res == SOCKET_ERROR)
				{
					std::cerr << "Failed to set socket as non-blocking. Error code: "
							<< res << std::endl;
					closesocket(tempSock);
					WSACleanup();
					exit(1);
				}

				// unlocks when out of scope
				std::unique_lock<std::shared_mutex> lock(_sessionMutex);

				// Add to session map
				_sessions.insert({clientState.playerId, clientState});
			}
			else
			{
				std::cerr << "Failed to send player ID to new client" << std::endl;
				closesocket(tempSock);
			}
			tempSock = INVALID_SOCKET;

		}
		else
		{
			std::cerr << "client connection failed :( " << WSAGetLastError() << std::endl;
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
            std::cerr << "WARNING: Select returned error in read thread: "
                    << WSAGetLastError() << std::endl;
        }
        else
        {
            std::shared_lock<std::shared_mutex> lock(_sessionMutex);

            // Iterate over sessions
            for (auto& pair : _sessions)
            {
                SocketState session = pair.second;

                // If in the read set, recv and push to buffer
                if (FD_ISSET(session.socket, &readSet))
                {
                    int recvResult = recv(
                            session.socket,
                            session.readBuf.data(),
                            (int)session.readBuf.size(),
                            0);

                    // If zero, we had a clean disconnect, so mark as dead
                    if (!recvResult)
                    {
                        // Mark socket as dead
                        sessionsToKill.push(session.playerId);
                    }
                    else if (recvResult != SOCKET_ERROR)
                    {
                        session.bytesRead += recvResult;

                        // If we're not in reading mode and received at least
                        // four bytes, store length and go into reading mode.
                        if (!session.isReading && session.bytesRead >= sizeof(uint32_t))
                        {
                            // Get the first four bytes and store as length
                            memcpy(&(session.length), session.readBuf.data(), sizeof(uint32_t));
                            session.isReading = true;

                            // Remove size from beginning of buffer
                            session.bytesRead -= sizeof(uint32_t);
                            session.readBuf.erase(session.readBuf.begin(), session.readBuf.begin() + sizeof(uint32_t));
                        }

                        // Deserialize object
                        if (session.isReading && session.bytesRead >= session.length)
                        {
                            ss.write(session.readBuf.data(), session.length);
                            cereal::BinaryInputArchive iarchive(ss);
                            std::shared_ptr<GameEvent> eventPtr;
                            iarchive(eventPtr);

                            // Lock and add to queue
                            std::unique_lock<std::mutex> eventLock(_eventMutex);
                            _eventQueue->push(eventPtr);
                            eventLock.unlock();
                        }
                    }
                    else
                    {
                        // Otherwise we had a miscellaneous error. Mark as dead.
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {
                            std::cerr << "Encountered error while reading socket for player "
                                    << session.playerId << " with code " << WSAGetLastError()
                                    << ". Closing player session." << std::endl;
                            sessionsToKill.push(session.playerId);
                        }
                    }
                }

                // If in the exception set, mark session as dead.
                if (FD_ISSET(session.socket, &exceptSet))
                {
                    // Mark socket as dead 
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        std::cerr << "Encountered error while reading socket for player "
                                << session.playerId << " with code " << WSAGetLastError()
                                << ". Closing player session." << std::endl;
                        sessionsToKill.push(session.playerId);
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

    std::stringstream ss;
    while (true)
    {
        // this is blocking
        std::shared_ptr<BaseState> nextItem = _updateQueue->pop();
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(nextItem);
        ss.seekg(0, std::ios::end);
        uint32_t size = (uint32_t)ss.tellg();
        ss.seekg(0, std::ios::beg);

        char * databuf = (char*)malloc(size);

        ss.read(databuf, size);

        std::shared_lock<std::shared_mutex> lock(_sessionMutex);
        for (auto& pair : _sessions)
        {
            SocketState session = pair.second;
            std::copy(databuf, databuf + size, std::back_inserter(session.writeBuf));
        }

        std::cerr << "Bufsize: " << size;
         
        free(databuf);
    }
    return;
}


void NetworkServer::closePlayerSession(uint32_t playerId)
{
    std::unique_lock<std::shared_mutex> lock(_sessionMutex);

    // Get a copy of the session that we're going to close
    auto result = _sessions.find(playerId);

    // If found, close the socket and remove from the map.
    if (result != _sessions.end())
    {
        std::cerr << "Closing session for player " << playerId
                << std::endl;
        closesocket(result->second.socket);
        _sessions.erase(result);
    }
    else
    {
        std::cerr << "Failed to find session for player " << playerId
                << std::endl;
    }
}

std::vector<std::shared_ptr<GameEvent>> NetworkServer::receiveEvents()
{
	// TODO: return contents of event queue
	return std::vector<std::shared_ptr<GameEvent>>();
}


void NetworkServer::sendUpdates(std::vector<std::shared_ptr<BaseState>> updates)
{
    for (auto& update : updates)
    {
        _updateQueue->push(update);
    }
}
