#pragma once
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

#include "Shared/BaseState.hpp"

struct PlayerState : BaseState
{
	// TODO: player-specific variables

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(cereal::base_class<BaseState>(this));
	}
};

// Register the polymorphic struct with cereal
#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(PlayerState);