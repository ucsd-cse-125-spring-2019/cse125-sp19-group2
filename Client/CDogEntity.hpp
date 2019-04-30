#pragma once

#include "CPlayerEntity.hpp"

class CDogEntity : public CPlayerEntity
{
public:
	CDogEntity() : CPlayerEntity()
	{
		_playerModel = std::make_unique<Model>("./Resources/Models/dog.fbx");
	};
	~CDogEntity() {};
};

