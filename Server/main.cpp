#include <cstdio>

#include "Shared/PlayerState.h"
#include "SOIL.h"

int main(int argc, char ** argv)
{
	PlayerState * ps = new PlayerState;
	ps->helloworld();
	fgetc(stdin);
}
