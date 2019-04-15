#pragma once

#include "Shared/ExampleState.hpp"
#include "SBaseEntity.hpp"

/*
** Example server-side entity. Remove as you wish.
*/
class SExampleEntity : public SBaseEntity
{
private:
    std::shared_ptr<ExampleState> _state;
public:
    SExampleEntity()
    {
        // Create a nonsensical state struct for example's sake
        _state = std::make_shared<ExampleState>();
        _state->type = ENTITY_EXAMPLE;
        _state->id = 1;
        _state->pos = glm::vec3(1);
        _state->up = glm::vec3(2);
        _state->normal = glm::vec3(3);
        _state->scale = 0.5;
        _state->extraVar = 123; // State specific to ExampleState
    };
    ~SExampleEntity() {};

    void update() { return; }
    std::shared_ptr<BaseState> getState() { return _state; }
};
