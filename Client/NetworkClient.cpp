#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>

#include "NetworkClient.hpp"

NetworkClient::NetworkClient()
{
	_updateQueue = std::make_unique<std::queue<std::shared_ptr<BaseState>>>();
	_eventQueue = std::make_unique<BlockingQueue<std::shared_ptr<GameEvent>>>();
	_socket = INVALID_SOCKET;
}


NetworkClient::~NetworkClient() {}


uint32_t NetworkClient::connect(std::string address, std::string port)
{
	SOCKET clientSock = INVALID_SOCKET;

	// Address information
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;

	WSADATA wsaData;
	int iResult;

	// Initialize Winsock version 2.2
	Logger::getInstance()->debug("Initializing Winsock");

	if ((iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		// NOTE: Since Winsock failed to load we cannot use 
		// WSAGetLastError to determine the specific error for
		// why it failed. Instead we can rely on the return 
		// status of WSAStartup.

		throw std::runtime_error("WSAStartup failed with error: " + iResult);
	}

	// Fill addr info struct
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Get address info from hints
	if ((iResult = getaddrinfo(address.c_str(), port.c_str(), &hints, &result) != 0))
	{
		WSACleanup();
		throw std::runtime_error("getAddrInfo failed with error: " + iResult);
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) 
	{
		// Create a SOCKET for connecting to server
		clientSock = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
		if (clientSock == INVALID_SOCKET) 
		{
			WSACleanup();
			throw std::runtime_error("Socket creation failed with error: " + WSAGetLastError());
		}

		// Disable nagle's algorithm
		char one = 1;
		setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

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
		throw std::runtime_error("Failed to connect to server");
	}
	else
	{
		// Assign socket
		_socket = clientSock;

		// Get player id from socket
		int bytesRead = 0;
		char playerId[sizeof(uint32_t)];
		while (bytesRead < sizeof(uint32_t))
		{
			int recvResult = recv(
					_socket,
					playerId + bytesRead,
					sizeof(playerId) - bytesRead,
					0);

			// We got data
			if (recvResult > 0)
			{
				bytesRead += recvResult;
			}
			else
			{
				// Otherwise we had an error
				closesocket(_socket);
				_socket = INVALID_SOCKET;
				throw(std::runtime_error("Error when receiving playerId"));
			}
		}

		// Threads are dependent on this variable to run
		_isAlive = true;

		// Spawn threads for server I/O
		_readThread = std::thread(
				&NetworkClient::socketReadHandler,
				this);
		_writeThread = std::thread(
				&NetworkClient::socketWriteHandler,
				this);

		Logger::getInstance()->info(
				"Successfully connected to " + address + ":" + port +
				" with playerId " + std::to_string(*(uint32_t*)playerId));

		// Return player ID
		return *(uint32_t*)playerId;
	}
}


void NetworkClient::socketReadHandler()
{
	// Buffer for recv
	char * readBuf = (char*)calloc(1, RECV_BUFSIZE);

	// Iterate until thread interrupted
	while (_isAlive)
	{
		uint32_t bytesRead = 0;

		// Get length of object following
		char lengthBuf[sizeof(uint32_t)];
		while (bytesRead < sizeof(uint32_t))
		{
			int recvResult = recv(
					_socket,
					lengthBuf + bytesRead,
					sizeof(uint32_t) - bytesRead,
					0);
			if (recvResult > 0)
			{
				bytesRead += recvResult;
			}
			else if (!recvResult || recvResult == SOCKET_ERROR)
			{
				Logger::getInstance()->info(
						"Failed to read from socket, " +
						std::string("shutting down read thread"));

				closeConnection();
				free(readBuf);
				return;
			}
		}

		// Grab length from buffer
		uint32_t length = *lengthBuf;

		// Grab next "length" bytes
		bytesRead = 0;

		// Read until we've buffered the expected object
		while (bytesRead < length)
		{
			int recvResult = recv(
					_socket,
					readBuf + bytesRead,
					length - bytesRead,
					0);

			if (recvResult > 0)
			{
				bytesRead += recvResult;
			}
			else if (!recvResult || recvResult == SOCKET_ERROR)
			{
				Logger::getInstance()->info(
						"Failed to read from socket, " +
						std::string("shutting down read thread"));

				closeConnection();
				free(readBuf);
				return;
			}
		}

		// Stream for deserialization
		std::stringstream ss;

		// Deserialize the object
		ss.write(readBuf, RECV_BUFSIZE);
		cereal::BinaryInputArchive iarchive(ss);
		std::shared_ptr<BaseState> statePtr;
		iarchive(statePtr);

		// Lock and add to queue
		std::unique_lock<std::mutex> lock(_updateMutex);
		_updateQueue->push(statePtr);
		lock.unlock();
	}

	free(readBuf);
	return;
}


void NetworkClient::socketWriteHandler()
{
	// Buffer for send
	char * sendBuf = (char*)calloc(1, SEND_BUFSIZE);

	while (_isAlive)
	{
		// Grab item off of event queue. Will block if queue is empty
		std::shared_ptr<GameEvent> nextItem;
		_eventQueue->pop(nextItem);

		// Create an output archive and serialize the object from the queue
		std::stringstream ss;
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(nextItem);

		// Get the size of the serialized object
		ss.seekg(0, std::ios::end);
		uint32_t size = (uint32_t)ss.tellg();
		ss.seekg(0, std::ios::beg);

		// Copy into send buffer
		// Size of serialized object first, then object itself
		memcpy(sendBuf, &size, sizeof(uint32_t));
		ss.read(sendBuf + sizeof(uint32_t), size);

		// Send raw data to the socket
		int bytesSent = 0;
		while (bytesSent != size + sizeof(uint32_t))
		{
			int sendResult = send(
				_socket,
				sendBuf + bytesSent,
				size + sizeof(uint32_t) - bytesSent,
				0);

			if (sendResult > 0)
			{
				bytesSent += sendResult;
			}
			else if (!sendResult || sendResult == SOCKET_ERROR)
			{
				// Debug statement, may be removed as necessary
				Logger::getInstance()->info(
						"Failed to write to socket, " +
						std::string("shutting down write thread"));

				closeConnection();
				free(sendBuf);
				return;
			}
		}
	}

	free(sendBuf);
	return;
}


void NetworkClient::closeConnection()
{
	std::unique_lock<std::mutex> lock(_socketMutex, std::defer_lock);

	if (!lock.try_lock())
	{
		return;
	}

	if (_socket != INVALID_SOCKET)
	{
		Logger::getInstance()->info("Closing connection to server");

		// Shutdown threads
		_isAlive = false;

		// Shutdown socket
		shutdown(_socket, SD_BOTH);

		// Add dummy item to event queue to unblock write thread
		auto dummyPtr = std::make_shared<GameEvent>();
		_eventQueue->push(dummyPtr);

		// Wait for threads to shutdown
		if (std::this_thread::get_id() != _readThread.get_id())
		{
			_readThread.join();
		}
		if (std::this_thread::get_id() != _writeThread.get_id())
		{
			_writeThread.join();
		}

		// Empty queues
		while (!_eventQueue->isEmpty())
		{
			std::shared_ptr<GameEvent> eventPtr;
			_eventQueue->pop(eventPtr);
		}
		std::unique_lock<std::mutex> queueLock(_updateMutex);
		while (!_updateQueue->empty())
		{
			_updateQueue->pop();
		}

		// Close socket and clean up winsock env
		closesocket(_socket);
		WSACleanup(); // If issues, remove this
		_socket = INVALID_SOCKET;

		Logger::getInstance()->info(
				"Successfully disconnected from server");
	}
}


void NetworkClient::sendEvents(std::vector<std::shared_ptr<GameEvent>> events)
{
	// Check to see if connection is active, throw exception if it is not
	std::unique_lock<std::mutex> socketLock(_socketMutex);
	if (_socket == INVALID_SOCKET)
	{
		socketLock.unlock();
		throw(std::runtime_error("Not connected to a server"));
	}
	else
	{
		socketLock.unlock();
		for (auto& event : events)
		{
			_eventQueue->push(event);
		}
	}
}


void NetworkClient::sendEvent(std::shared_ptr<GameEvent> event)
{
	// Check to see if connection is active, throw exception if it is not
	std::unique_lock<std::mutex> socketLock(_socketMutex);
	if (_socket == INVALID_SOCKET)
	{
		socketLock.unlock();
		throw(std::runtime_error("Not connected to a server"));
	}
	else
	{
		socketLock.unlock();
		_eventQueue->push(event);
	}
}


std::vector<std::shared_ptr<BaseState>> NetworkClient::receiveUpdates()
{
	// Check to see if connection is active, throw exception if it is not
	std::unique_lock<std::mutex> socketLock(_socketMutex);
	if (_socket == INVALID_SOCKET)
	{
		socketLock.unlock();
		throw(std::runtime_error("Not connected to a server"));
	}
	else
	{
		socketLock.unlock();

		auto updateList = std::vector<std::shared_ptr<BaseState>>();

		// Lock and build updates from queue
		std::unique_lock<std::mutex> lock(_updateMutex);
		while (!_updateQueue->empty())
		{
			updateList.push_back(_updateQueue->front());
			_updateQueue->pop();
		}
		return updateList;
	}
}
