#pragma once
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

#include "Shared/BaseState.hpp"

struct GateState : BaseState
{
	int jailIndex;
	float liftRate;

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			cereal::base_class<BaseState>(this),
			jailIndex,
			liftRate);
	}
};

// Register the polymorphic struct with cereal
#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(GateState);