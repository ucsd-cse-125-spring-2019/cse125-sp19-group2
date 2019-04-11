#include "NetworkServer.hpp"

NetworkServer::NetworkServer(std::string address, uint8_t port)
{
	// Initialize queues
	_updateQueue = std::make_unique<BlockingQueue<std::shared_ptr<BaseState>>>();
	_eventQueue = std::make_unique<std::queue<std::shared_ptr<GameEvent>>>();

	// Start listener thread
	_listener = std::thread(&NetworkServer::connectionListener, this, MAX_CONNECTIONS);
}


NetworkServer::~NetworkServer()
{
	// TODO: destroy the queues
}


void NetworkServer::connectionListener(uint8_t maxConnections)
{
	// TODO: listen and spawn new thread for each player
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