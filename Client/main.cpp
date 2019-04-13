#include <cstdio>
#include <memory>
#include <SOIL.h>

#include "Application.hpp"
#include "CExampleEntity.hpp"
#include "NetworkClient.hpp"

int main(int argc, char ** argv)
{
    uint32_t res;
	NetworkClient* newClient = new NetworkClient();

    // Attempt connection
    try
    {
        res = newClient->connect("localhost", PORTNUM);
    }
    catch (std::runtime_error e)
    {
        std::cerr << e.what() << std::endl;
        fgetc(stdin);
        return 1;
    }

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
     
    // Try to get updates from the server
	std::this_thread::sleep_for(std::chrono::seconds(5));

    try
    {
        for (auto& item : newClient->receiveUpdates()) {
            item->print();
        }
    }
    catch (std::runtime_error e)
    {
        // Not connected to the server
        std::cerr << e.what() << std::endl;
    }

	Application * app = new Application("ProjectBone", argc, argv);
	app->Run();
}

