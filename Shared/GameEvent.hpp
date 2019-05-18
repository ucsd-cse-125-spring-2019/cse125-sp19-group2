#pragma once

#include <glm/glm.hpp>
#include <stdint.h>
#include <string>
#include "Shared/Logger.hpp"

// Allow Cereal serialization of GLM 2-item vectors
namespace cereal
{
	template<class Archive>
	void serialize(Archive & archive, glm::vec2 &v)
	{
        archive(v.x, v.y);
	}
}

/*
** A few enums outlining the different types of events that could be triggered
** by a player. As of now this would end up being a large case-switch on the
** server side. If we want, we could make this a little cleaner with
** polymorphism.
*/
enum EventType
{
    EVENT_PLAYER_JOIN,
    EVENT_PLAYER_SWITCH,
	EVENT_PLAYER_READY,
    EVENT_PLAYER_LEAVE, // Created only by the server
    EVENT_PLAYER_MOVE,	// Movement vector for player
    EVENT_PLAYER_RUN_START,	// Used only for dogs at the moment
    EVENT_PLAYER_RUN_END,	// Used only for dogs at the moment
    EVENT_PLAYER_STOP,
    EVENT_PLAYER_URINATE_START,
    EVENT_PLAYER_URINATE_END,
    EVENT_PLAYER_LIFTING_START,
    EVENT_PLAYER_LIFTING_END
    // TODO: more event types here
};
// GamePad Indexes
typedef enum
{
	Player_Dog = 0,
	Player_Human = 1,
}PlayerType;
/*
** This is a very basic struct to get us off the ground with game events. If
** we need more functionality, we can expand this as necessary.
*/
struct GameEvent
{
    EventType type;
    uint32_t playerId; // Not used for EVENT_PLAYER_JOIN
    std::string playerName; // Used only for EVENT_PLAYER_JOIN
	glm::vec2 direction;	// Used only for PLAYER_TURN and PLAYER_MOVE
    //TODO: add more elements as necessary

	// Serialization for Cereal
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(
			type,
			playerId,
			playerName,
			direction);
    }

	void print()
	{
		Logger::getInstance()->debug("[playerEvent type: " + std::to_string(type) + " playerID: " + std::to_string(playerId) + " playerName: " + playerName + "]");
	}
};

