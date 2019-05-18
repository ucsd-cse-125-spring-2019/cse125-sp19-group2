#pragma once
#include "CBaseEntity.hpp"
#include "Model.hpp"
#include "Camera.hpp"
#include "Animation.hpp"
#include "glad/glad.h"
#include "InputManager.h"
#include "GuiManager.hpp"
#include "Shared/PlayerState.hpp"

class CPlayerEntity : public CBaseEntity
{
public:
	CPlayerEntity()
	{
		// Allocate member variables
		_objectShader = std::make_unique<Shader>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/animation.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/animation.frag");
		_objectShader->CreateProgram();
	}

	void render(std::unique_ptr<Camera> const& camera) override
	{
		// Update Animation
		_objectModel->update();

		// Base render
		CBaseEntity::render(camera);
	}

	virtual void updateState(std::shared_ptr<BaseState> state) override
	{
		// Base update first
		CBaseEntity::updateState(state);

		// Invert z
		_state->forward.z = -_state->forward.z;

		// Player-specific stuff
		auto currentState = std::static_pointer_cast<PlayerState>(_state);
		auto newState = std::static_pointer_cast<PlayerState>(state);

		// TODO: player variables
	}

	std::shared_ptr<BaseState> const& getState() { return _state; }

	void setLocal(bool flag)
	{
		_isLocal = flag;
	}

	virtual glm::vec3 getPos() const override
	{
		if (_isLocal)
		{
			glm::vec3 pos = _state->pos;
			pos.y += _state->height * 0.9;
			return pos;
		}
		else
		{
			return CBaseEntity::getPos();
		}
	}

protected:
	void initAnimation(std::string modelPath)
	{
		// Read in an animated Mesh
		_objectModel = std::make_unique<Animation>(modelPath);

		// Cast model as animation
		Animation* animation = static_cast<Animation*>(_objectModel.get());

		// Ensuring index is non-garbage value
		animation->animatedMesh->_takeIndex = 0;

		// Call init to let Animation precache uniform location
		animation->init(_objectShader);

		// Set Animation to playing mode
		animation->_isPlaying = true;
	}

	bool _isLocal = false;
};
