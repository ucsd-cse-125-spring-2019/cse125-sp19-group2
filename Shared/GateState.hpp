#pragma once
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

#include "Shared/BaseState.hpp"

class GateState : BaseState
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

