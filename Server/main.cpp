#include <cstdio>
#include <iostream>

#include "Shared/GameEvent.hpp"
#include "GameServer.hpp"

int main(int argc, char ** argv)
{
    GameServer * server = new GameServer();
    server->start();
}
