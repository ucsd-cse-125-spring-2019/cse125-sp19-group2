#pragma once

#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

#include "Shared/BaseState.hpp"

/*
** Example state class. Remove as you wish.
*/
struct ExampleState : BaseState
{
    int extraVar;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(cereal::base_class<BaseState>(this), extraVar);
    }
};

// Register the polymorphic struct with cereal
#include <cereal/archives/binary.hpp>
CEREAL_REGISTER_TYPE(ExampleState);
