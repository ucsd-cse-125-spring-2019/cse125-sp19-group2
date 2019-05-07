#include "LocalPlayer.hpp"
#include "InputManager.h"
#include "Shared/GameEvent.hpp"
#include "EntityManager.hpp"

LocalPlayer::LocalPlayer(uint32_t playerId, std::unique_ptr<NetworkClient> const& networkClient) {

    _playerId = playerId;

    // Player move forward event
    InputManager::getInstance().getKey(GLFW_KEY_W)->onRepeat(
        [&] {
            auto event = std::make_shared<GameEvent>();
            event->playerId = _playerId;
            event->type = EVENT_PLAYER_MOVE_FORWARD;

            // Try sending the update
            try {
                networkClient->sendEvent(event);
            }
            catch (std::runtime_error e) {
            };
        });

    // Player move backward event
    InputManager::getInstance().getKey(GLFW_KEY_S)->onRepeat(
        [&] {
            auto event = std::make_shared<GameEvent>();
            event->playerId = _playerId;
            event->type = EVENT_PLAYER_MOVE_BACKWARD;

            // Try sending the update
            try {
                networkClient->sendEvent(event);
            }
            catch (std::runtime_error e) {
            };
        });

    // Player move left event
    InputManager::getInstance().getKey(GLFW_KEY_A)->onRepeat(
        [&] {
            auto event = std::make_shared<GameEvent>();
            event->playerId = _playerId;
            event->type = EVENT_PLAYER_MOVE_LEFT;

            // Try sending the update
            try {
                networkClient->sendEvent(event);
            }
            catch (std::runtime_error e) {
            };
        });

    // Player move right event
    InputManager::getInstance().getKey(GLFW_KEY_D)->onRepeat(
        [&] {
            auto event = std::make_shared<GameEvent>();
            event->playerId = _playerId;
            event->type = EVENT_PLAYER_MOVE_RIGHT;

            // Try sending the update
            try {
                networkClient->sendEvent(event);
            }
            catch (std::runtime_error e) {
            };
        });

    _camera = std::make_unique<Camera>();
    _camera->set_pitch(_pitch);
    _offset = glm::normalize(glm::vec3(0.0f, 0.5f, 0.5f));
}

void LocalPlayer::update() {
    if (!_playerEntity) {
        _playerEntity = std::dynamic_pointer_cast<CPlayerEntity>(EntityManager::getInstance().getEntity(_playerId));

        // Break out if player entity does not yet exist
        if (!_playerEntity) {
            return;
        }
        _playerEntity->setLocal(true);

        InputManager::getInstance().getKey(85)->onPress(
            [this]() {
                _playerEntity->getPlayerModel()->animatedMesh->_takeIndex += 1;
                _playerEntity->getPlayerModel()->animatedMesh->_takeIndex %= _playerEntity
                                                                             ->
                                                                             getPlayerModel()
                                                                             ->animatedMesh
                                                                             ->takeCount();
            });

        GuiManager::getInstance().getFormHelper("Form helper")->addGroup("Cycle Animation");
        GuiManager::getInstance().getFormHelper("Form helper")->addButton(
            "Next", [&]() {
                _playerEntity->getPlayerModel()->animatedMesh->_takeIndex += 1;
                _playerEntity->getPlayerModel()->animatedMesh->_takeIndex %= _playerEntity
                                                                             ->
                                                                             getPlayerModel()
                                                                             ->animatedMesh->
                                                                             takeCount();
                _playerEntity->setCurrentAnim(
                    _playerEntity->getPlayerModel()->animatedMesh->getCurrentAnimName());
            })->setTooltip(
            "Testing a much longer tooltip, that will wrap around to new lines multiple times.");

        GuiManager::getInstance().getFormHelper("Form helper")->addVariable(
            "Current Animation", _playerEntity->getCurrentAnim());

        GuiManager::getInstance().setDirty();
    }
    _camera->set_position(_playerEntity->getState()->pos + _distance * _offset);
    _camera->Update();
}

std::unique_ptr<Camera> const& LocalPlayer::getCamera() {
    return _camera;
}

void LocalPlayer::resize(int x, int y) {
    const float aspect = float(x) / y;
    float fovx = glm::degrees(2 * atan(tan(glm::radians(45 * 0.5)) * aspect));
    if (fovx >= 45) {
        // update fovy as well
        fovx = 45;
        const float newFovy = glm::degrees(2 * atan(tan(glm::radians(45 * 0.5)) / aspect));
        _camera->set_fovy(newFovy);
    }
    else {
        _camera->set_fovy(45.0f);
    }
    _camera->set_aspect(aspect);
}
