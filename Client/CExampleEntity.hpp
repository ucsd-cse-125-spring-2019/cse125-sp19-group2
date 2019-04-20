#pragma once

#include <memory>

#include "CBaseEntity.hpp"
#include "Shared/ExampleState.hpp"

/*
** Example client-side object. Remove as you wish.
*/

class CExampleEntity : CBaseEntity
{
private:
    ExampleState _state;

public:
    void render() { return; }

    void updateState(std::shared_ptr<BaseState> state)
    {
        std::shared_ptr<ExampleState> castState =
            std::static_pointer_cast<ExampleState>(state);

        // This is a little messy, so in the future we might want a copy
        // constructor inside the struct (if that's possible)
        _state.forward = castState->forward;
        _state.up = castState->up;
        _state.pos = castState->pos;
        _state.scale = castState->scale;
        _state.extraVar = castState->extraVar;
    };

    uint32_t getId() { return _state.id; }
};
