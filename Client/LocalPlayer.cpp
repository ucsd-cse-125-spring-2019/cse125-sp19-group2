#include "LocalPlayer.hpp"
#include "InputManager.h"
#include "Shared/GameEvent.hpp"
#include "EntityManager.hpp"
#include <glm/gtx/string_cast.hpp>

LocalPlayer::LocalPlayer(uint32_t playerId, std::unique_ptr<NetworkClient> const& networkClient)
{
	_playerId = playerId;

	_pad = std::make_unique<GamePadXbox>(GamePadIndex_One);

	// Player move forward event
	InputManager::getInstance().getKey(GLFW_KEY_W)->onRepeat([&]
	{
		_moveKeysPressed = true;
		_stopped = false;
		auto event = std::make_shared<GameEvent>();
		event->playerId = _playerId;
		event->type = EVENT_PLAYER_MOVE;
		event->direction = _camera->convert_direction(glm::vec2(0, -1));

		// Try sending the update
		try
		{
			networkClient->sendEvent(event);
		}
		catch (std::runtime_error e)
		{
		};
	});

	// Player move backward event
	InputManager::getInstance().getKey(GLFW_KEY_S)->onRepeat([&]
	{
		_moveKeysPressed = true;
		_stopped = false;
		auto event = std::make_shared<GameEvent>();
		event->playerId = _playerId;
		event->type = EVENT_PLAYER_MOVE;
		event->direction = _camera->convert_direction(glm::vec2(0, 1));

		// Try sending the update
		try
		{
			networkClient->sendEvent(event);
		}
		catch (std::runtime_error e)
		{
		};
	});

	// Player move left event
	InputManager::getInstance().getKey(GLFW_KEY_A)->onRepeat([&]
	{
		_moveKeysPressed = true;
		_stopped = false;
		auto event = std::make_shared<GameEvent>();
		event->playerId = _playerId;
		event->type = EVENT_PLAYER_MOVE;
		event->direction = _camera->convert_direction(glm::vec2(-1, 0));

		// Try sending the update
		try
		{
			networkClient->sendEvent(event);
		}
		catch (std::runtime_error e)
		{
		};
	});

	// Player move right event
	InputManager::getInstance().getKey(GLFW_KEY_D)->onRepeat([&]
	{
		_moveKeysPressed = true;
		_stopped = false;
		auto event = std::make_shared<GameEvent>();
		event->playerId = _playerId;
		event->type = EVENT_PLAYER_MOVE;
		event->direction = _camera->convert_direction(glm::vec2(1, 0));

		// Try sending the update
		try
		{
			networkClient->sendEvent(event);
		}
		catch (std::runtime_error e)
		{
		};
	});

	InputManager::getInstance().onScroll([&](float y)
	{
		_camera->set_distance(-y);
	});

	InputManager::getInstance().getKey(GLFW_MOUSE_BUTTON_RIGHT)->onPress([&]
	{
		glfwSetInputMode(InputManager::getInstance().getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		_moveCamera = true;
	});

	InputManager::getInstance().getKey(GLFW_MOUSE_BUTTON_RIGHT)->onRelease([&]
	{
		glfwSetInputMode(InputManager::getInstance().getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		_moveCamera = false;
	});

	InputManager::getInstance().getKey2D(Key::KEYTYPE::MOUSE)->onMove([&](glm::vec2 v)
	{
		//std::cout << glm::to_string(v) << std::endl;
		if (_moveCamera && InputManager::getInstance().isForegroundWindow())
		{
			_camera->move_camera(v);
		}
	});

	InputManager::getInstance().getKey2D(Key::KEYTYPE::RSTICK)->onMove([&](glm::vec2 v)
	{
		//std::cout << glm::to_string(v) << std::endl;
		auto vec = v;
		vec.y = -vec.y;
		_camera->move_camera(vec * 20.0f);
	});

	InputManager::getInstance().getKey2D(Key::KEYTYPE::LSTICK)->onMove([&](glm::vec2 v)
	{
		//std::cout << glm::to_string(v) << std::endl;

		// Invert z
		v.y = -(v.y);

		// Send event
		auto event = std::make_shared<GameEvent>();
		event->type = EVENT_PLAYER_MOVE;
		event->playerId = _playerId;
		event->direction = _camera->convert_direction(v);
		std::cout << glm::to_string(event->direction) << std::endl;
		_networkClient->sendEvent(event);

		_stopped = false;
		_moveKeysPressed = true;
	});

	// Player move right event
	InputManager::getInstance().getKey(GLFW_KEY_LEFT_SHIFT)->onRepeat([&]
	{
		auto event = std::make_shared<GameEvent>();
		event->playerId = _playerId;
		event->type = EVENT_PLAYER_RUN;

		// Try sending the update
		try
		{
			networkClient->sendEvent(event);
		}
		catch (std::runtime_error e)
		{
		};
	});

	_camera = std::make_unique<Camera>();

	_networkClient = networkClient.get();

	_moveKeysPressed = false;
	_stopped = true;
	_moveCamera = false;

	// TODO: set player model height properly
	_height = 1.0f;
}

void LocalPlayer::update()
{
	if (!_playerEntity)
	{
		_playerEntity = std::dynamic_pointer_cast<CPlayerEntity>(EntityManager::getInstance().getEntity(_playerId));

		// Break out if player entity does not yet exist
		if (!_playerEntity)
		{
			return;
		}
		_playerEntity->setLocal(true);

		GuiManager::getInstance().setDirty();
	}
	glm::vec3 pos = _playerEntity->getState()->pos;
	pos.y += _height;
	_camera->set_position(pos);
	_camera->Update();

	// Stop events for the server
	if (!_stopped && !_moveKeysPressed)
	{
		_stopped = true;
		auto event = std::make_shared<GameEvent>();
		event->playerId = _playerId;
		event->type = EVENT_PLAYER_STOP;
		_networkClient->sendEvent(event);
	}

	_moveKeysPressed = false;
}

void LocalPlayer::updateController()
{
	if (_pad->isConnected())
	{
		auto left = _pad->getLS();
		InputManager::getInstance().move(Key::KEYTYPE::LSTICK, left.x, left.y);


		auto right = _pad->getRS();
		InputManager::getInstance().move(Key::KEYTYPE::RSTICK, right.x, right.y);


		if (_pad->isKeyDown(GamePad_Button_DPAD_LEFT))
		{
			InputManager::getInstance().fire(GLFW_KEY_A, KeyState::Press);
		};
		if (_pad->isKeyUp(GamePad_Button_DPAD_LEFT))
		{
			InputManager::getInstance().fire(GLFW_KEY_A, KeyState::Release);
		}
		if (_pad->isKeyDown(GamePad_Button_DPAD_RIGHT))
		{
			InputManager::getInstance().fire(GLFW_KEY_D, KeyState::Press);
		}
		if (_pad->isKeyUp(GamePad_Button_DPAD_RIGHT))
		{
			InputManager::getInstance().fire(GLFW_KEY_D, KeyState::Release);
		}
		if (_pad->isKeyDown(GamePad_Button_DPAD_UP))
		{
			InputManager::getInstance().fire(GLFW_KEY_W, KeyState::Press);
		}
		if (_pad->isKeyUp(GamePad_Button_DPAD_UP))
		{
			InputManager::getInstance().fire(GLFW_KEY_W, KeyState::Release);
		}
		if (_pad->isKeyDown(GamePad_Button_DPAD_DOWN))
		{
			InputManager::getInstance().fire(GLFW_KEY_S, KeyState::Press);
		}
		if (_pad->isKeyUp(GamePad_Button_DPAD_DOWN))
		{
			InputManager::getInstance().fire(GLFW_KEY_S, KeyState::Release);
		}
	}
}

std::unique_ptr<Camera> const& LocalPlayer::getCamera()
{
	return _camera;
}

void LocalPlayer::resize(int x, int y)
{
	_camera->resize(x, y);
}
