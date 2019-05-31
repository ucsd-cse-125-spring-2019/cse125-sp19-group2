#include <cstdio>
#include <iostream>

#include "Shared/GameEvent.hpp"
#include "GameServer.hpp"

static GameServer* server = nullptr;

// Callback for window close
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	if (fdwCtrlType == CTRL_CLOSE_EVENT ||
		fdwCtrlType == CTRL_C_EVENT)
	{
		if (server)
		{
			server->shutdown();
			return true;
		}
	}
	return false;
}

int main(int argc, char ** argv)
{
	// Register callback for window close
	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	// Create and start server
    server = new GameServer();
    server->start();
}
