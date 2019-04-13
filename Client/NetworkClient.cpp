#include "NetworkClient.hpp"

NetworkClient::NetworkClient()
{
	// TODO: init queues
}


NetworkClient::~NetworkClient()
{
	// TODO: Destroy queues
}


uint32_t NetworkClient::connect(std::string address, std::string port)
{
	// TODO: connect to IP and port, create socket and spawn socketHandler()
	SOCKET clientSock = INVALID_SOCKET;

	// Address information
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;

	WSADATA wsaData;
	int iResult;

	// Initialize Winsock version 2.2
	OutputDebugString("Initializing Winsock\n");

	if ((iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		// NOTE: Since Winsock failed to load we cannot use 
		// WSAGetLastError to determine the specific error for
		// why it failed. Instead we can rely on the return 
		// status of WSAStartup.

		OutputDebugString( "WSAStartup failed with error: " + iResult);
	}

	// Fill addr info struct
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Get address info from hints
	OutputDebugString("Attempt to Get address info\n");
	// TODO: change default port to port
	if ((iResult = getaddrinfo(address.c_str(), port.c_str(), &hints, &result) != 0))
	{
		OutputDebugString("getaddrinfo failed with error: " + iResult);
		WSACleanup();
		return 1;
	}
	OutputDebugString("Attempt to connect\n");
	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) 
	{
		// Create a SOCKET for connecting to server
		clientSock = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (clientSock == INVALID_SOCKET) 
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = ::connect(clientSock, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) 
		{
			closesocket(clientSock);
			clientSock = INVALID_SOCKET;
			continue;
		}
		break;
	}
	if (clientSock == INVALID_SOCKET) 
	{
		OutputDebugString("failed to connect to any port");
		return 1;
	}
	else
	{
		/*if (ptr->ai_family == AF_INET)
		{
			std::cerr << "Connected to server at "
					<< ptr->ai_addrlen << " on port " << port
					<< std::endl;
		}
		else 
		{
			OutputDebugString("connected to the port" + ((struct sockaddr_in6*)ptr)->sin6_port);
		}*/

		// Assign socket
		_socket = clientSock;

		// Get player id from socket
		int bytesRead = 0;
		char playerId[sizeof(uint32_t)];
		while (bytesRead < sizeof(uint32_t))
		{
			int recvResult = recv(
					_socket,
					playerId,
					sizeof(playerId),
					0);

			// We got data
			if (recvResult > 0)
			{
				bytesRead += recvResult;
			}
			else
			{
				// Otherwise we had an error
				std::cerr << "Error when receiving playerId" << std::endl;
				closesocket(_socket);
				_socket = INVALID_SOCKET;
				return 1;
			}
		}

		// Spawn threads for server I/O
		_readThread = std::thread(
				&NetworkClient::socketReadHandler,
				this);
		_writeThread = std::thread(
				&NetworkClient::socketWriteHandler,
				this);

		// Return player ID
		return (uint32_t)*playerId;
	}
	// thread
	return 1;
}

void NetworkClient::socketReadHandler()
{
	return;
}


void NetworkClient::socketWriteHandler()
{
	return;
}


void NetworkClient::sendEvents(std::vector<std::shared_ptr<GameEvent>>)
{
	// TODO: add to event queue
}


std::vector<std::shared_ptr<BaseState>> NetworkClient::receiveUpdates()
{
	// TODO: remove from update queue
	return std::vector<std::shared_ptr<BaseState>>();
}
