#pragma once

#include "CBaseEntity.hpp"
#include "Model.hpp"

class CFountainEntity : public CBaseEntity
{
public:
	CFountainEntity() {
		_objectModel = std::make_unique<Model>("./Resources/Models/fountain.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};
	~CFountainEntity() {};

	void updateState(std::shared_ptr<BaseState> state) override {
		CBaseEntity::updateState(state);

		if (!_waterSound) {
			_waterSound = AudioManager::getInstance().getAudioSource("fountain sound" + std::to_string(state->id));
			_waterSound->init("Resources/Sounds/fountain.wav", true, true);
			_waterSound->setVolume(0.4f);
			_waterSound->setPosition(state->pos);
			_waterSound->play(true);
		}
	}

protected:
	AudioSource * _waterSound = nullptr;
};

