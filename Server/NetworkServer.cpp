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

		std::unique_lock<std::mutex> lock(_sessionMutex);
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
				std::vector<byte>(), // read buffer
				false, // is reading
				0, // length (bytes to read)
				0, // bytes read
				std::vector<byte>() // write buffer
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
				std::unique_lock<std::mutex> lock(_sessionMutex);

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


void NetworkServer::socketHandler()
{
	// Read socket set for select()
	FD_SET readSet;

	while (true)
	{
		// Reset readSet and add sockets from session map
		FD_ZERO(&readSet);
		std::unique_lock<std::mutex> lock(_sessionMutex);
		for (auto& pair : _sessions)
		{
			FD_SET(pair.second.socket, &readSet);
		}

		// Unlock and wait for read activity
		lock.unlock();
		// TODO
	}
}


std::vector<std::shared_ptr<GameEvent>> NetworkServer::receiveEvents()
{
	// TODO: return contents of event queue
	return std::vector<std::shared_ptr<GameEvent>>();
}


void NetworkServer::sendUpdates(std::vector<std::shared_ptr<BaseState>>)
{
	
	// TODO: add to update queue
}