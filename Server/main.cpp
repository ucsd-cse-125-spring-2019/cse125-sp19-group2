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

    std::cout << "Hello World!" << std::endl;
	fgetc(stdin);
}
