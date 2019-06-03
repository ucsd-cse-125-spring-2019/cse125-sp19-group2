#pragma once
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

#include "Shared/BaseState.hpp"

struct PlungerState : BaseState
{
	bool isStuck;	// Whether the plunger has stuck to anything yet

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			cereal::base_class<BaseState>(this),
			isStuck);
	}
};

// Register the polymorphic struct with cereal
#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(PlungerState);
