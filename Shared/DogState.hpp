#pragma once
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

#include "Shared/PlayerState.hpp"

struct DogState : PlayerState
{
	DogAnimation currentAnimation;
	float runStamina;
	float urineMeter;
	bool isCaught;
	bool isBarking;
	bool isTeleporting;	// Used only for teleporting sound

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			cereal::base_class<PlayerState>(this),
			currentAnimation,
			runStamina,
			urineMeter,
			isCaught,
			isBarking,
			isTeleporting);
	}
};

// Register the polymorphic struct with cereal
#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(DogState);