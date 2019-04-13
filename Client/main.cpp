#include <cstdio>
#include <memory>
#include <SOIL.h>

#include "Application.hpp"
#include "CExampleEntity.hpp"
#include "NetworkClient.hpp"

int main(int argc, char ** argv)
{
	OutputDebugString("start\n");
	NetworkClient* newClient = new NetworkClient();
	auto res = newClient->connect("localhost", PORTNUM);
	std::cerr << "Our player ID is " << res << std::endl;

	/*
	std::this_thread::sleep_for(std::chrono::seconds(5));
	auto results = newClient->receiveUpdates();
	std::cerr << results.size();
	*/

    // Sample client-side object. Remove as you wish. You probably want these
    // kinds of objects inside Application.cpp, but it's here for reference.
    // Unique_ptr's might also make more sense as well.
    std::shared_ptr<CExampleEntity> exampleEntity = std::make_shared<CExampleEntity>();

    // When server returns a vector of BaseState structs, and one of their IDs
    // matches our exampleEntity, we update it:
    std::shared_ptr<BaseState> baseStatePtr = std::make_shared<ExampleState>();
    exampleEntity->updateState(baseStatePtr);


	std::this_thread::sleep_for(std::chrono::seconds(5));
	for (auto& item : newClient->receiveUpdates()) {
		item->print();
	}

	newClient->closeConnection();

	std::this_thread::sleep_for(std::chrono::seconds(5));

	newClient->connect("localhost", PORTNUM);

	Application * app = new Application("ProjectBone", argc, argv);
	app->Run();
}

