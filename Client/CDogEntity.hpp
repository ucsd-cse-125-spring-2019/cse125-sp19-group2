#pragma once

#include "CPlayerEntity.hpp"

class CDogEntity : public CPlayerEntity
{
public:
    CDogEntity() : CPlayerEntity()
    {
        // Load Animation
        initAnimation("./Resources/Models/dog.dae");
    };
    ~CDogEntity() {};
};

