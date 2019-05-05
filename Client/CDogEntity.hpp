#pragma once

#include "CPlayerEntity.hpp"

class CDogEntity : public CPlayerEntity
{
public:
	CDogEntity() : CPlayerEntity()
	{
		// Load Animation
        init("./Resources/Models/dog.dae");
	};
	~CDogEntity() {};
};

