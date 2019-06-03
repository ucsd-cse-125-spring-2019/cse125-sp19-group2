#pragma once
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

#include "Shared/BaseState.hpp"

struct GateState : BaseState
{
	bool isLifting;	// Whether the gates are moving up or not

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			cereal::base_class<BaseState>(this),
			isLifting);
	}
};

#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(GateState);