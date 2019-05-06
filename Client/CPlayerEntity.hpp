﻿#pragma once
#include "CBaseEntity.hpp"
#include "Model.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.hpp"
#include "Animation.hpp"
#include "glad/glad.h"
#include "InputManager.h"
#include "GuiManager.hpp"

class CPlayerEntity : public CBaseEntity {
public:
    CPlayerEntity() {
        // Allocate member variables
        _playerShader = std::make_unique<Shader>();
        _state = std::make_shared<BaseState>();

        _playerShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/animation.vert");
        _playerShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/animation.frag");
        _playerShader->CreateProgram();

        // Read in an animated Mesh
        _playerModel = std::make_unique<Animation>(_modelPath);

		InputManager::getInstance().getKey(85)->onPress([this]()
		{
			_playerModel->animatedMesh->_takeIndex += 1;
			_playerModel->animatedMesh->_takeIndex %= _playerModel->animatedMesh->takeCount();
		});

        GuiManager::getInstance().getFormHelper("Form helper")->addGroup("Cycle Animation");
        GuiManager::getInstance().getFormHelper("Form helper")->addButton("Next", [&]() {
            _playerModel->animatedMesh->_takeIndex += 1;
		    _playerModel->animatedMesh->_takeIndex %= _playerModel->animatedMesh->takeCount();
            _currentAnim = _playerModel->animatedMesh->getCurrentAnimName();
        })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");

        GuiManager::getInstance().getFormHelper("Form helper")->addVariable("Current Animation", _currentAnim);

        GuiManager::getInstance().setDirty();

        // TODO: determine which animation gets played
        _playerModel->animatedMesh->_takeIndex = 0;

        // Call init to let Animation precache uniform location
		_playerModel->init(_playerShader);

        // Set Animation to playing mode
		_playerModel->_isPlaying = true;
    }

    void render(std::unique_ptr<Camera> const& camera) override {
		// Update Animation
		_playerModel->update();

        if(_isLocal) {
			//return;
        }

        _playerShader->Use();
        _playerShader->set_uniform("u_projection", camera->projection_matrix());
        _playerShader->set_uniform("u_view", camera->view_matrix());

        // Compute model matrix based on state: t * r * s
        const auto t = glm::translate(glm::mat4(1.0f), _state->pos);
		const auto r = glm::lookAt(glm::vec3(0.0f),_state->forward, _state->up);
        const auto s = glm::scale(glm::mat4(1.0f), glm::vec3(0.099f * 5.0f)); 
        
		auto model = t * r * s;
        // Pass model matrix into shader
        _playerShader->set_uniform("u_model", model);
        _playerShader->set_uniform("u_material.shininess", 0.6f);
        _playerShader->set_uniform("u_pointlight.position", glm::vec3(-3.0f, 3.0f, -3.0f));
        _playerShader->set_uniform("u_pointlight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        _playerShader->set_uniform("u_pointlight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        _playerShader->set_uniform("u_pointlight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        _playerShader->set_uniform("u_pointlight.constant", 1.0f);
        _playerShader->set_uniform("u_pointlight.linear", 0.09f);
        _playerShader->set_uniform("u_pointlight.quadratic", 0.032f);
		_playerShader->set_uniform("u_numpointlights", static_cast<GLuint>(1));

        // Render the animated mesh
        _playerModel->render(_playerShader);
    }

    void updateState(std::shared_ptr<BaseState> state) override {
        //_state = state;
        _state->id = state->id;

        // Translation
        _state->pos = state->pos;

        // Rotation
        _state->forward = state->forward;
		_state->forward.z = -_state->forward.z;
        _state->up = state->up;

        // Scale
        _state->scale = state->scale;

    }

    uint32_t getId() override {
        return _state->id;
    }

    std::shared_ptr<BaseState> const& getState() { return _state; }

    void setLocal(bool flag) {
		_isLocal = flag;
    }

    void setPath(const std::string & filepath) {
        _modelPath = filepath;
    }

protected:
	bool _isLocal = false;

    std::shared_ptr<BaseState> _state;
    std::unique_ptr<Animation> _playerModel;
    std::unique_ptr<Shader> _playerShader;
    std::string _modelPath = "./Resources/Models/dog.dae";
    std::string _currentAnim = "";
};
