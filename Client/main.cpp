#include <cstdio>
#include <memory>
#include <SOIL.h>

#include "Application.hpp"
#include "CExampleEntity.hpp"

int main(int argc, char ** argv)
{
    // Sample client-side object. Remove as you wish. You probably want these
    // kinds of objects inside Application.cpp, but it's here for reference.
    // Unique_ptr's might also make more sense as well.
    std::shared_ptr<CExampleEntity> exampleEntity = std::make_shared<CExampleEntity>();

    // When server returns a vector of BaseState structs, and one of their IDs
    // matches our exampleEntity, we update it:
    std::shared_ptr<BaseState> baseStatePtr = std::make_shared<ExampleState>();
    exampleEntity->updateState(baseStatePtr);

	Application * app = new Application("ProjectBone", argc, argv);
	app->Run();
}

