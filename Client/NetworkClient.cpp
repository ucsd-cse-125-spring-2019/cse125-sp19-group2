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
