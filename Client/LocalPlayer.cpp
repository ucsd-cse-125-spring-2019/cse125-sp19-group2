#include "LocalPlayer.hpp"
#include "InputManager.h"
#include "Shared/GameEvent.hpp"
#include "EntityManager.hpp"

LocalPlayer::LocalPlayer(uint32_t playerId, std::unique_ptr<NetworkClient> const& networkClient) {

    _playerId = playerId;

	// Spawn threads for server I/O
	_readThread = std::thread(
		&LocalPlayer::inputReadHandler,
		this);

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
void LocalPlayer::inputReadHandler()
{
	GamePadXbox* pad = new GamePadXbox(GamePadIndex_One);

	while (1)
	{
		if (pad->is_connected())
		{
			if (pad->KeyDown(GamePad_Button_DPAD_LEFT)) {
				InputManager::getInstance().fire(GLFW_KEY_A, KeyState::Press);
			};
			if (pad->KeyUp(GamePad_Button_DPAD_LEFT)) {
				InputManager::getInstance().fire(GLFW_KEY_A, KeyState::Release);
			}
			if (pad->KeyDown(GamePad_Button_DPAD_RIGHT)) {
				InputManager::getInstance().fire(GLFW_KEY_D, KeyState::Press);
			}
			if (pad->KeyUp(GamePad_Button_DPAD_RIGHT)) {
				InputManager::getInstance().fire(GLFW_KEY_D, KeyState::Release);
			}
			if (pad->KeyDown(GamePad_Button_DPAD_UP)) {
				InputManager::getInstance().fire(GLFW_KEY_W, KeyState::Press);
			}
			if (pad->KeyUp(GamePad_Button_DPAD_UP)) {
				InputManager::getInstance().fire(GLFW_KEY_W, KeyState::Release);
			}
			if (pad->KeyDown(GamePad_Button_DPAD_DOWN)) {
				InputManager::getInstance().fire(GLFW_KEY_S, KeyState::Press);
			}
			if (pad->KeyUp(GamePad_Button_DPAD_DOWN)) {
				InputManager::getInstance().fire(GLFW_KEY_S, KeyState::Release);
			}
		}
	}
	delete pad;
}
std::unique_ptr<Camera> const& LocalPlayer::getCamera() {
    return _camera;
}
