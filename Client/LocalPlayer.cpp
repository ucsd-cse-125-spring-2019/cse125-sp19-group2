#include "LocalPlayer.hpp"
#include "InputManager.h"
#include "Shared/GameEvent.hpp"
#include "EntityManager.hpp"

LocalPlayer::LocalPlayer(uint32_t playerId, std::unique_ptr<NetworkClient> const& networkClient) {

    _playerId = playerId;

    // Player move forward event
    InputManager::getInstance().getKey(GLFW_KEY_W)->onRepeat([&]
    {
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
    InputManager::getInstance().getKey(GLFW_KEY_S)->onRepeat([&]
    {
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
    InputManager::getInstance().getKey(GLFW_KEY_A)->onRepeat([&]
    {
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
    InputManager::getInstance().getKey(GLFW_KEY_D)->onRepeat([&]
    {
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

	// Player move right event
	InputManager::getInstance().getKey(GLFW_KEY_SHIFT)->onRepeat([&]
	{
		auto event = std::make_shared<GameEvent>();
		event->playerId = _playerId;
		event->type = EVENT_PLAYER_RUN;

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
    }
    _camera->set_position(_playerEntity->getState()->pos + _distance * _offset);
    _camera->Update();
}

std::unique_ptr<Camera> const& LocalPlayer::getCamera() {
    return _camera;
}
