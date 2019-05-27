#pragma once

#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

#include "Shared/PlayerState.hpp"

struct HumanState : public PlayerState
{
	// TODO: Human-specific state
	HumanAnimation currentAnimation;
	float chargeMeter;
	long plungerCooldown;	// Milliseconds until plunger is usable
	long trapCooldown;	// Milliseconds until trap is usable

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			cereal::base_class<PlayerState>(this),
			currentAnimation,
			chargeMeter,
			plungerCooldown,
			trapCooldown);
	}
};

// Register the polymorphic struct with cereal
#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(HumanState);