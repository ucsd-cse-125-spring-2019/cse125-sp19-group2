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
	bool pregameCountdown;
	long millisecondsToStart;	// Countdown to game start
	long millisecondsLeft;	// Time remaining in game before dogs win
	long millisecondsToLobby;	// Time remaining before everyone is returned to the lobby
	EntityType winner;	// Either ENTITY_DOG or ENTITY_HUMAN
	std::unordered_map<uint32_t, std::string> dogs;
	std::unordered_map<uint32_t, std::string> humans;
	std::vector<uint32_t> readyPlayers;	// Players ready to start the game

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			cereal::base_class<BaseState>(this),
			gameStarted,
			gameOver,
			inLobby,
			pregameCountdown,
			millisecondsToStart,
			millisecondsLeft,
			millisecondsToLobby,
			winner,
			dogs,
			humans,
			readyPlayers);
	}

	// Some items used by server, not serialized for the client
	std::chrono::time_point<std::chrono::steady_clock> _pregameStart;
	std::chrono::time_point<std::chrono::steady_clock> _gameStart;
	std::chrono::time_point<std::chrono::steady_clock> _endgameStart;
	std::chrono::nanoseconds _gameDuration;
};

// Register polymorphic relationship with cereal
#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(GameState);
