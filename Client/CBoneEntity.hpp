#pragma once

#include "CBaseEntity.hpp"
#include "Model.hpp"

class CBoneEntity : public CBaseEntity
{
public:
	CBoneEntity() {
		// Allocate member variables
		_objectModel = std::make_unique<Model>("./Resources/Models/dogbone.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};

	~CBoneEntity() {};

	virtual void updateState(std::shared_ptr<BaseState> state) override
	{
		CBaseEntity::updateState(state);

		// If slated to be destroyed, play eating sound
		if (!_state->isDestroyed && state->isDestroyed)
		{
			auto eatingSound = AudioManager::getInstance().getAudioSource("dog eating" + std::to_string(_state->id));
			eatingSound->init("Resources/Sounds/dog_eating.wav", false, true);
			eatingSound->setPosition(_state->pos);
			eatingSound->setVolume(0.3f);
			eatingSound->play(true);
		}

		_state->isDestroyed = state->isDestroyed;
	}
};
