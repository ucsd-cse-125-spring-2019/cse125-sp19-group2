#include "NetworkClient.hpp"

NetworkClient::NetworkClient()
{
	// TODO: init queues
}


NetworkClient::~NetworkClient()
{
	// TODO: Destroy queues
}


void NetworkClient::socketHandler(SOCKET socket)
{
	// TODO: add to update queue, remove from event queue
}

uint32_t NetworkClient::connect(std::string address, uint8_t port)
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

	// Setup Winsock communication code here 

	// When your application is finished call WSACleanup
	if (WSACleanup() == SOCKET_ERROR)
	{
		printf("WSACleanup failed with error %d\n", WSAGetLastError());
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
	if ((iResult = getaddrinfo(address.c_str(), "8080", &hints, &result) != 0))
	{
		OutputDebugString("getaddrinfo failed with error: " + iResult);
		WSACleanup();
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
	}
	else
	{
		if (ptr->ai_family == AF_INET)
		{
			OutputDebugString("connected to the port" + ((struct sockaddr_in*)ptr)->sin_port);
		}
		else 
		{
			OutputDebugString("connected to the port" + ((struct sockaddr_in6*)ptr)->sin6_port);
		}
	}
	// thread
	return 0;
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
