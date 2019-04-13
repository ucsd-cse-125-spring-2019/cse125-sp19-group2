#include <cstdio>
#include <memory>
#include <SOIL.h>

#include "Application.hpp"

int main(int argc, char ** argv)
{
	Application * app = new Application("ProjectBone", argc, argv);
	app->Run();
}

