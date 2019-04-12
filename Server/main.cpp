#include <cstdio>
#include <iostream>
#include <memory>
#include <SOIL.h>
#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>

#include "SExampleEntity.hpp"
#include "NetworkServer.hpp"

int main(int argc, char ** argv)
{
    // Example serialization and deserialization
   /* std::stringstream ss;
    {
        cereal::BinaryOutputArchive oarchive(ss);

        // Example of server-side entity with state
        std::shared_ptr<SBaseEntity> exampleObject = std::make_shared<SExampleEntity>();

        oarchive(exampleObject->getState());
    }
    {
        cereal::BinaryInputArchive iarchive(ss);

        // Deserialize state info
        std::shared_ptr<BaseState> state;
        iarchive(state);

        // It should be an ExampleState
        std::cout << typeid(*state).name() << std::endl;

        // Cast as exampleState and get var
        std::shared_ptr<ExampleState> castState = std::static_pointer_cast<ExampleState>(state);

        // It shouldn't be garbage
        std::cout << castState->extraVar << std::endl;
    }
	*/

	NetworkServer * server = new NetworkServer(PORTNUM);

    std::shared_ptr<BaseState> testState = std::make_shared<BaseState>();
    testState->id = 5;
    testState->scale = 2.5;

    for (int i = 0; i < 5; i++)
    {
        std::vector<std::shared_ptr<BaseState>> states = std::vector<std::shared_ptr<BaseState>>();
        states.push_back(testState);
        server->sendUpdates(states);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    std::cout << "Hello World!" << std::endl;
	fgetc(stdin);
}
