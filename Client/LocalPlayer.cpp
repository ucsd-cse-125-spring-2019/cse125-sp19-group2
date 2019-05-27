#include "LocalPlayer.hpp"
#include "InputManager.h"
#include "Shared/GameEvent.hpp"
#include "EntityManager.hpp"
#include "AudioManager.hpp"
#include <glm/gtx/string_cast.hpp>

LocalPlayer::LocalPlayer(uint32_t playerId, std::unique_ptr<NetworkClient> const& networkClient) {
    _playerId = playerId;

    _gamePad = std::make_unique<GamePadXbox>(GamePadIndex_NULL);

    // Player move forward event
    InputManager::getInstance().getKey(GLFW_KEY_W)->onRepeat(
        [&] {
            _moveKeysPressed = true;
            _stopped = false;
            auto event = std::make_shared<GameEvent>();
            event->playerId = _playerId;
            event->type = EVENT_PLAYER_MOVE;
            event->direction = _camera->convert_direction(glm::vec2(0, -1));

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
            _moveKeysPressed = true;
            _stopped = false;
            auto event = std::make_shared<GameEvent>();
            event->playerId = _playerId;
            event->type = EVENT_PLAYER_MOVE;
            event->direction = _camera->convert_direction(glm::vec2(0, 1));

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
            _moveKeysPressed = true;
            _stopped = false;
            auto event = std::make_shared<GameEvent>();
            event->playerId = _playerId;
            event->type = EVENT_PLAYER_MOVE;
            event->direction = _camera->convert_direction(glm::vec2(-1, 0));

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
            _moveKeysPressed = true;
            _stopped = false;
            auto event = std::make_shared<GameEvent>();
            event->playerId = _playerId;
            event->type = EVENT_PLAYER_MOVE;
            event->direction = _camera->convert_direction(glm::vec2(1, 0));

            // Try sending the update
            try {
                networkClient->sendEvent(event);
            }
            catch (std::runtime_error e) {
            };
        });

    InputManager::getInstance().onScroll(
        [&](float y) {
            _camera->set_distance(-y);
        });

	// Right mouse button for skill
    InputManager::getInstance().getKey(GLFW_MOUSE_BUTTON_RIGHT)->onPress(
        [&] {
			auto event = std::make_shared<GameEvent>();
			event->playerId = _playerId;
			event->type = EVENT_PLAYER_URINATE_START;
			try
			{
				networkClient->sendEvent(event);
			}
			catch (std::runtime_error e)
			{
			};

			// For human only
			if (_usePlunger)
			{
				event = std::make_shared<GameEvent>();
				event->playerId = _playerId;
				event->type = EVENT_PLAYER_LAUNCH_START;
				event->direction = _camera->convert_direction(glm::vec2(0, -1));
				try
				{
					networkClient->sendEvent(event);
				}
				catch (std::runtime_error e)
				{
				};
			}
			else
			{
				event = std::make_shared<GameEvent>();
				event->playerId = _playerId;
				event->type = EVENT_PLAYER_PLACE_TRAP;
				try
				{
					networkClient->sendEvent(event);
				}
				catch (std::runtime_error e)
				{
				};
			}
        });

    InputManager::getInstance().getKey(GLFW_MOUSE_BUTTON_RIGHT)->onRelease(
        [&] {
			auto event = std::make_shared<GameEvent>();
			event->playerId = _playerId;
			event->type = EVENT_PLAYER_URINATE_END;
			try
			{
				networkClient->sendEvent(event);
			}
			catch (std::runtime_error e)
			{
			};

			// Plunger release
			event = std::make_shared<GameEvent>();
			event->playerId = _playerId;
			event->type = EVENT_PLAYER_LAUNCH_END;
			try
			{
				networkClient->sendEvent(event);
			}
			catch (std::runtime_error e)
			{
			};
        });

    InputManager::getInstance().getKey2D(Key::KEYTYPE::MOUSE)->onMove(
        [&](glm::vec2 v) {
            //std::cout << glm::to_string(v) << std::endl;
            if (_moveCamera && InputManager::getInstance().isForegroundWindow()) {
                _camera->move_camera(v);
			}
        });

    InputManager::getInstance().getKey2D(Key::KEYTYPE::RSTICK)->onMove(
        [&](glm::vec2 v) {
            //std::cout << glm::to_string(v) << std::endl;
            auto vec = v;
            vec.y = -vec.y;

			// Scale look movement with time
            _camera->move_camera(vec * ((float)GuiManager::getInstance().getLastFrameLength() / 1000));
        });

    InputManager::getInstance().getKey2D(Key::KEYTYPE::LSTICK)->onMove(
        [&](glm::vec2 v) {
            //std::cout << glm::to_string(v) << std::endl;

            // Invert z
            v.y = -(v.y);

            // Send event
            auto event = std::make_shared<GameEvent>();
            event->type = EVENT_PLAYER_MOVE;
            event->playerId = _playerId;
            event->direction = _camera->convert_direction(v);
            _networkClient->sendEvent(event);

            _stopped = false;
            _moveKeysPressed = true;
        });

   // Dog sprinting start
	InputManager::getInstance().getKey(GLFW_KEY_LEFT_SHIFT)->onPress([&]
	{
		auto event = std::make_shared<GameEvent>();
		event->playerId = _playerId;
		event->type = EVENT_PLAYER_RUN_START;

		// Try sending the update
		try
		{
			networkClient->sendEvent(event);
		}
		catch (std::runtime_error e)
		{
		};
	});

	// Dog sprinting finish
	InputManager::getInstance().getKey(GLFW_KEY_LEFT_SHIFT)->onRelease([&]
	{
		auto event = std::make_shared<GameEvent>();
		event->playerId = _playerId;
		event->type = EVENT_PLAYER_RUN_END;

		// Try sending the update
		try
		{
			networkClient->sendEvent(event);
		}
		catch (std::runtime_error e)
		{
		};
	});

	InputManager::getInstance().getKey(GLFW_MOUSE_BUTTON_LEFT)->onPress([&]
	{
		// We don't want to register clicks if the mouse isn't captured
		if (_moveCamera)
		{
			// Humans swinging nets
			auto event = std::make_shared<GameEvent>();
			event->playerId = _playerId;
			event->type = EVENT_PLAYER_CHARGE_NET;
			try
			{
				networkClient->sendEvent(event);
			}
			catch (std::runtime_error e)
			{
			};

			// Dogs interacting
			event = std::make_shared<GameEvent>();
			event->playerId = _playerId;
			event->type = EVENT_PLAYER_INTERACT_START;

			try
			{
				networkClient->sendEvent(event);
			}
			catch (std::runtime_error e)
			{
			};
		}
	});

	InputManager::getInstance().getKey(GLFW_MOUSE_BUTTON_LEFT)->onRelease([&]
	{
		// We don't want to register clicks if the mouse isn't captured
		if (_moveCamera)
		{
			// Humans swinging nets
			auto event = std::make_shared<GameEvent>();
			event->playerId = _playerId;
			event->type = EVENT_PLAYER_SWING_NET;
			try
			{
				networkClient->sendEvent(event);
			}
			catch (std::runtime_error e)
			{
			};

			// Dogs interacting
			event = std::make_shared<GameEvent>();
			event->playerId = _playerId;
			event->type = EVENT_PLAYER_INTERACT_END;
			try
			{
				networkClient->sendEvent(event);
			}
			catch (std::runtime_error e)
			{
			};
		}
	});

	// Q to switch skills
	InputManager::getInstance().getKey(GLFW_KEY_Q)->onPress([&]
	{
		_usePlunger = !_usePlunger;
		GuiManager::getInstance().setActiveSkill(_usePlunger);
	});

    _camera = std::make_unique<Camera>();

    _networkClient = networkClient.get();

    _moveKeysPressed = false;
    _stopped = true;
    _moveCamera = false;

    // TODO: set player model height properly
    _height = 1.0f;
}

void LocalPlayer::update() {
    if (!_playerEntity) {
        _playerEntity = std::dynamic_pointer_cast<CPlayerEntity>(EntityManager::getInstance().getEntity(_playerId));

        // Break out if player entity does not yet exist
        if (!_playerEntity) {
            return;
        }
        _playerEntity->setLocal(true);
        if (_playerEntity->getState()->type == ENTITY_HUMAN) {
            _camera->set_heightfactor(0.8);
        }
        else if (_playerEntity->getState()->type == ENTITY_DOG) {
            _camera->set_heightfactor(0.63);
        }
        _height = _playerEntity->getState()->height * 0.9f;
    }
    glm::vec3 pos = _playerEntity->getState()->pos;
    pos.y += _height;
    _camera->set_position(pos);

    _camera->Update();

	// Update audio engine
	if (_playerEntity)
	{
		AudioManager::getInstance().setListenerPos(_playerEntity->getState()->pos);
		auto forward2D = _camera->convert_direction(glm::vec2(0, -1));
		auto forward3D = glm::vec3(forward2D.x, 0, forward2D.y);
		AudioManager::getInstance().setListenerDir(forward3D);
	}

    // Stop events for the server
    if (!_stopped && !_moveKeysPressed) {
        _stopped = true;
        auto event = std::make_shared<GameEvent>();
        event->playerId = _playerId;
        event->type = EVENT_PLAYER_STOP;
        _networkClient->sendEvent(event);
    }

    _moveKeysPressed = false;
}

void LocalPlayer::updateController() {
    if (_gamePad->isConnected()) {
        auto left = _gamePad->getLS();
        InputManager::getInstance().move(Key::KEYTYPE::LSTICK, left.x, left.y);

        auto right = _gamePad->getRS();
        InputManager::getInstance().move(Key::KEYTYPE::RSTICK, right.x, right.y);

        auto lt = _gamePad->getForceLT();
        auto rt = _gamePad->getForceRT();
        auto s = rt - lt;

		// Scale trigger by time
		float scaledTrigger = s * ((float)GuiManager::getInstance().getLastFrameLength() / 50000);
        InputManager::getInstance().scroll(scaledTrigger);

        if (_gamePad->isKeyDown(GamePad_Button_DPAD_LEFT)) {
            InputManager::getInstance().fire(GLFW_KEY_A, KeyState::Press);
        };
        if (_gamePad->isKeyUp(GamePad_Button_DPAD_LEFT)) {
            InputManager::getInstance().fire(GLFW_KEY_A, KeyState::Release);
        }
        if (_gamePad->isKeyDown(GamePad_Button_DPAD_RIGHT)) {
            InputManager::getInstance().fire(GLFW_KEY_D, KeyState::Press);
        }
        if (_gamePad->isKeyUp(GamePad_Button_DPAD_RIGHT)) {
            InputManager::getInstance().fire(GLFW_KEY_D, KeyState::Release);
        }
        if (_gamePad->isKeyDown(GamePad_Button_DPAD_UP)) {
            InputManager::getInstance().fire(GLFW_KEY_W, KeyState::Press);
        }
        if (_gamePad->isKeyUp(GamePad_Button_DPAD_UP)) {
            InputManager::getInstance().fire(GLFW_KEY_W, KeyState::Release);
        }
        if (_gamePad->isKeyDown(GamePad_Button_DPAD_DOWN)) {
            InputManager::getInstance().fire(GLFW_KEY_S, KeyState::Press);
        }
        if (_gamePad->isKeyUp(GamePad_Button_DPAD_DOWN)) {
            InputManager::getInstance().fire(GLFW_KEY_S, KeyState::Release);
        }

		// Map "B" on xbox to right click (use skill)
		if (_gamePad->isKeyDown(GamePad_Button_B)) {
			InputManager::getInstance().fire(GLFW_MOUSE_BUTTON_RIGHT, KeyState::Press);
		}
		if (_gamePad->isKeyUp(GamePad_Button_B)) {
			InputManager::getInstance().fire(GLFW_MOUSE_BUTTON_RIGHT, KeyState::Release);
		}

		// Map "X" on xbox to left shift (sprint for dogs)
		if (_gamePad->isKeyDown(GamePad_Button_X)) {
			InputManager::getInstance().fire(GLFW_KEY_LEFT_SHIFT, KeyState::Press);
		}
		if (_gamePad->isKeyUp(GamePad_Button_X)) {
			InputManager::getInstance().fire(GLFW_KEY_LEFT_SHIFT, KeyState::Release);
		}

		// Map "A" on xbox to left click (interact / charge & swing net)
		if (_gamePad->isKeyDown(GamePad_Button_A)) {
			InputManager::getInstance().fire(GLFW_MOUSE_BUTTON_LEFT, KeyState::Press);
		}
		if (_gamePad->isKeyUp(GamePad_Button_A)) {
			InputManager::getInstance().fire(GLFW_MOUSE_BUTTON_LEFT, KeyState::Release);
		}

		// Map both bumpers to "Q" to switch skills on the human
		if (_gamePad->isKeyDown(GamePad_Button_LEFT_THUMB)) {
			if (!_leftBumperDown) {
				_leftBumperDown = true;
				InputManager::getInstance().fire(GLFW_KEY_Q, KeyState::Press);
			}
		}
		else {
			_leftBumperDown = false;
			InputManager::getInstance().fire(GLFW_KEY_Q, KeyState::Release);
		}
		if (_gamePad->isKeyDown(GamePad_Button_RIGHT_THUMB)) {
			if (!_rightBumperDown) {
				_rightBumperDown = true;
				InputManager::getInstance().fire(GLFW_KEY_Q, KeyState::Press);
			}
		}
		else {
			_rightBumperDown = false;
			InputManager::getInstance().fire(GLFW_KEY_Q, KeyState::Release);
		}
    }
}

std::unique_ptr<Camera> const& LocalPlayer::getCamera() {
    return _camera;
}

bool LocalPlayer::setControllerNum(GamePadIndex player)
{
	return _gamePad->setControllerNum(player);
}
void LocalPlayer::unpairEntity() {
	_playerEntity = nullptr;
}
void LocalPlayer::setPlayerType(PlayerType typeNum) 
{
	_playerType = typeNum;
}
void LocalPlayer::setMouseCaptured(bool mouseCaptured)
{
	_moveCamera = mouseCaptured;

	if (!mouseCaptured) {
		glfwSetInputMode(
			InputManager::getInstance().getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else {
		glfwSetInputMode(
			InputManager::getInstance().getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}
bool LocalPlayer::getMouseCaptured()
{
	return _moveCamera;
}
PlayerType LocalPlayer::getPlayerType()
{
	return _playerType;
}
uint32_t LocalPlayer::getPlayerId()
{
	return _playerId;
}

void LocalPlayer::resize(int x, int y) {
    _camera->resize(x, y);
}
