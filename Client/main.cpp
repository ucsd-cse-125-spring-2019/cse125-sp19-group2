#include <cstdio>

#include "Shared/PlayerState.h"
#include "SOIL.h"
#include "Application.hpp"

int main(int argc, char ** argv)
{
	PlayerState * ps = new PlayerState;
	ps->helloworld();

	Application * app = new Application("ProjectBone", argc, argv);
	app->Run();
}

