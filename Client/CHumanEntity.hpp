#pragma once

#include "CPlayerEntity.hpp"

class CHumanEntity : public CPlayerEntity
{
public:
	CHumanEntity() : CPlayerEntity()
	{
		_playerModel = std::make_unique<Model>("./Resources/Models/human.fbx");
	};
	~CHumanEntity() {};
};

