#pragma once

#include <stdint.h>
#include <string>
#include "Shared/Logger.hpp"

/*
** A few enums outlining the different types of events that could be triggered
** by a player. As of now this would end up being a large case-switch on the
** server side. If we want, we could make this a little cleaner with
** polymorphism.
*/
enum EventType
{
    EVENT_PLAYER_JOIN,
    EVENT_PLAYER_LEAVE, // Created only by the server
    EVENT_PLAYER_MOVE_FORWARD,
    EVENT_PLAYER_MOVE_BACKWARD,
    EVENT_PLAYER_MOVE_LEFT,
    EVENT_PLAYER_MOVE_RIGHT
    // TODO: more event types here
};

/*
** This is a very basic struct to get us off the ground with game events. If
** we need more functionality, we can expand this as necessary.
*/
struct GameEvent
{
    EventType type;
    uint32_t playerId; // Not used for EVENT_PLAYER_JOIN
    std::string playerName; // Used only for EVENT_PLAYER_JOIN
    //TODO: add more elements as necessary

    // Serialization for Cereal
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(type, playerId, playerName);
    }

	void print()
	{
		Logger::getInstance()->debug("[playerEvent type: " + std::to_string(type) + " playerID: " + std::to_string(playerId) + " playerName: " + playerName + "]");
	}
};

