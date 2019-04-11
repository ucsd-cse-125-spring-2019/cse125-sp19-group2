#include "NetworkServer.hpp"

NetworkServer::NetworkServer(std::string port)
{
	// Initialize queues
	_updateQueue = std::make_unique<BlockingQueue<std::shared_ptr<BaseState>>>();
	_eventQueue = std::make_unique<std::queue<std::shared_ptr<GameEvent>>>();

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
}


void NetworkServer::socketHandler(SOCKET socket, uint32_t playerId)
{
	// TODO: handle player socket (empty update queue, fill event queue)
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