#pragma once

#include "CPlayerEntity.hpp"

class CHumanEntity : public CPlayerEntity
{
public:
	CHumanEntity() : CPlayerEntity()
	{
        // Load Animation
        init("./Resources/Models/human.dae");
	};
	~CHumanEntity() {};
};

