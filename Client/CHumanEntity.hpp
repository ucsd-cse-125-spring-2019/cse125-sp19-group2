#pragma once

#include "CPlayerEntity.hpp"
#include "Shared/HumanState.hpp"

class CHumanEntity : public CPlayerEntity {
public:
    CHumanEntity(uint32_t id, int skinID) : CPlayerEntity() {
        // Init player state
        _state = std::make_shared<HumanState>();

        // Load Animation based on skin
        std::string modelLoc = "./Resources/Models/human";
        int usedSkin = skinID % HUMAN_SKIN_AMOUNT;
        if (usedSkin != 0)
            modelLoc += std::to_string(usedSkin);
        modelLoc += ".dae";
        initAnimation(modelLoc);

        // Load positional sounds
        _runningSound = AudioManager::getInstance().getAudioSource("human running" + std::to_string(id));
        _runningSound->init("Resources/Sounds/human_running.wav", true, true);
        _runningSound->setVolume(0.1f);

        _swingSound = AudioManager::getInstance().getAudioSource("human swinging" + std::to_string(id));

        _flyingSound = AudioManager::getInstance().getAudioSource("human flying" + std::to_string(id));
        _flyingSound->init("Resources/Sounds/human_flying.wav", true, true);
        _flyingSound->setVolume(0.1f);

        _slippingSound = AudioManager::getInstance().getAudioSource("human slipping" + std::to_string(id));

		// arrow indicator for charging swing
		_arrowModel = std::make_unique<Model>("./Resources/Models/swing_arrow.fbx");
		_arrowShader = std::make_unique<Shader>();
		_arrowShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_arrowShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_arrowShader->CreateProgram();
    };

    ~CHumanEntity() {
    };

    void updateState(std::shared_ptr<BaseState> state) override {
        // Set generic stuff first
        CPlayerEntity::updateState(state);

        // Set human-specific state variables
        auto currentState = std::static_pointer_cast<HumanState>(_state);
        auto newState = std::static_pointer_cast<HumanState>(state);

        /** Sounds **/
        _runningSound->setPosition(_state->pos);
        _swingSound->setPosition(_state->pos);
        _flyingSound->setPosition(_state->pos);
        _slippingSound->setPosition(_state->pos);

        // Running
        _runningSound->play(newState->currentAnimation == ANIMATION_HUMAN_RUNNING);

        // Net swing
        if (newState->currentAnimation != currentState->currentAnimation &&
            (newState->currentAnimation == ANIMATION_HUMAN_SWINGING1 ||
                newState->currentAnimation == ANIMATION_HUMAN_SWINGING2 ||
                newState->currentAnimation == ANIMATION_HUMAN_SWINGING3)) {
            _swingSound->init("Resources/Sounds/swing1.wav", false, true);
            _swingSound->setVolume(0.3f);
            _swingSound->setPosition(_state->pos);
            _swingSound->play(true);
        }

        // Flying
        _flyingSound->play(newState->currentAnimation == ANIMATION_HUMAN_FLYING);

        // Slipping
        if (newState->currentAnimation != currentState->currentAnimation &&
            newState->currentAnimation == ANIMATION_HUMAN_SLIPPING) {
            _slippingSound->init("Resources/Sounds/human_slipping.wav", false, true);
            _slippingSound->setVolume(0.10f);
            _slippingSound->setPosition(_state->pos);
            _slippingSound->play(true);
        }

        // Requires a cast
        Animation* humanAnimation = static_cast<Animation*>(_objectModel.get());
        
        //if (newState->currentAnimation != currentState->currentAnimation) {
            //std::cout << "newState->currentAnimation: " << humanAnimations[newState->currentAnimation] << std::endl;
            
            if (newState->isPlayOnce) {
                humanAnimation->playOnce(humanAnimations[newState->currentAnimation], newState->animationDuration);
            }
            else {
                humanAnimation->play(humanAnimations[newState->currentAnimation]);
            }
        //}
        
        /** Animation **/
        currentState->currentAnimation = newState->currentAnimation;

        // Human attributes and items
        if (_isLocal) {
            // Update state
            currentState->plungerCooldown = newState->plungerCooldown;
            currentState->trapCooldown = newState->trapCooldown;
            currentState->chargeMeter = newState->chargeMeter;

            // Update HUD
            GuiManager::getInstance().updatePlunger(newState->plungerCooldown);
            GuiManager::getInstance().updateTrap(newState->trapCooldown);
            GuiManager::getInstance().updateCharge(currentState->chargeMeter / MAX_HUMAN_CHARGE);
        }
    }

	void renderArrow(std::unique_ptr<Camera> const& camera) {
		auto currentState = std::static_pointer_cast<HumanState>(_state);
		if (currentState->chargeMeter > 0) {
			std::cout << currentState->chargeMeter << std::endl;
			_arrowShader->Use();
			_arrowShader->set_uniform("u_projection", camera->projection_matrix());
			_arrowShader->set_uniform("u_view", camera->view_matrix());

			// Setting tranparency
			_arrowShader->set_uniform("u_transparency", arrowTransparency);

			// Compute model matrix based on state: t * r * s
			const auto t = glm::translate(glm::mat4(1.0f), _state->pos + glm::vec3(0,0.3f,0));
			const auto r = glm::lookAt(glm::vec3(0.0f), _state->forward, _state->up);
			const auto s = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 1, currentState->chargeMeter * 3.0f));

			auto model = t * r * s;

			// Pass model matrix into shader
			_arrowShader->set_uniform("u_model", model);
			_arrowShader->set_uniform("u_dirlight.direction", glm::vec3(0.0f, -1.0f, -0.4f));
			_arrowShader->set_uniform("u_dirlight.ambient", glm::vec3(0.2f, 0.2f, 0.3f));
			_arrowShader->set_uniform("u_dirlight.diffuse", glm::vec3(0.8f, 0.8f, 0.9f));
			_arrowShader->set_uniform("u_numdirlights", static_cast<GLuint>(1));
			_arrowModel->render(_arrowShader);
		}
	}

protected:
	AudioSource* _runningSound;
	AudioSource* _swingSound;
	AudioSource* _flyingSound;
	AudioSource* _slippingSound;

	std::unique_ptr<Model> _arrowModel;
	std::unique_ptr<Shader> _arrowShader;
	float arrowTransparency = 0.5f;
};
