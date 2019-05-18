#pragma once

#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <chrono>

#include "Shared/BaseState.hpp"

struct GameState : public BaseState
{
	// Stuff that the client uses
	// There are probably a few extra booleans than needed, but we want to make
	// sure state is exactly what we think it is
	bool gameStarted;
	bool gameOver;
	bool inLobby;
	int numReady;	// Number of players ready to start the game
	long millisecondsToStart;	// Countdown to game start
	long millisecondsLeft;	// Time remaining in game before dogs win
	EntityType winner;	// Either ENTITY_DOG or ENTITY_HUMAN
	std::unordered_map<uint32_t, std::string> dogs;
	std::unordered_map<uint32_t, std::string> humans;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			cereal::base_class<BaseState>(this),
			gameStarted,
			inLobby,
			numReady,
			millisecondsToStart,
			millisecondsLeft,
			winner,
			dogs,
			humans);
	}

	// Some items used by server, not serialized for the client
	std::chrono::time_point<std::chrono::steady_clock> _gameStart;
	std::chrono::nanoseconds _gameDuration;
};

// Register polymorphic relationship with cereal
#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(GameState);
