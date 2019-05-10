#include "GamePadXbox.hpp"

bool GamePadXbox::isConnected()
{
	_controllerStatePre = _controllerState;

	// clean the state
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

	// Get the state
	DWORD Result = XInputGetState(_controllerNum, &_controllerState);

	if (Result == ERROR_SUCCESS)	return true;
	else return false;
}

void GamePadXbox::setVibration(glm::vec2 lr)
{
	// Create a new Vibraton 
	XINPUT_VIBRATION vibration;

	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	// Set the Vibration Values
	vibration.wLeftMotorSpeed = lr.x * 65535; // use any value between 0-65535 here
	vibration.wRightMotorSpeed = lr.y * 65535; // use any value between 0-65535 here

	// Vibrate the controller
	XInputSetState((int)_controllerNum, &vibration);
}

void GamePadXbox::reset()
{
	// clean the state
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));
	// clean the pre state
	ZeroMemory(&_controllerStatePre, sizeof(XINPUT_STATE));
}

bool GamePadXbox::isKeyUp(int button)
{
	return !(_controllerState.Gamepad.wButtons & button) &&
		(_controllerStatePre.Gamepad.wButtons & button);
}

bool GamePadXbox::isKeyDown(int button)
{
	return (_controllerState.Gamepad.wButtons & button) &&
		!(_controllerStatePre.Gamepad.wButtons & button);
}

bool GamePadXbox::getKeyState(int button)
{
	return _controllerState.Gamepad.wButtons & button;
}
float GamePadXbox::getForceLT()
{
	if (_controllerState.Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		return 0;
	return _controllerState.Gamepad.bLeftTrigger / 255.0f;
}


float GamePadXbox::getForceRT()
{
	BYTE ret = _controllerState.Gamepad.bRightTrigger;
	if (ret < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		return 0;
	return ret / 255.0f;
}

glm::vec2 GamePadXbox::getLS()
{
	float LX = _controllerState.Gamepad.sThumbLX;
	float LY = _controllerState.Gamepad.sThumbLY;

	if (abs(LX) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		LX = 0;
	if (abs(LY) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		LY = 0;
	if (LX == 0 && LY == 0)
		return glm::vec2(0, 0);
	//determine how far the controller is pushed
	float magnitude = sqrt(LX * LX + LY * LY);

	//determine the direction the controller is pushed
	float normalizedLX = LX / magnitude;
	float normalizedLY = LY / magnitude;

	return glm::vec2(normalizedLX, normalizedLY);
}

glm::vec2 GamePadXbox::getRS()
{
	float RX = _controllerState.Gamepad.sThumbRX;
	float RY = _controllerState.Gamepad.sThumbRY;

	if (abs(RX) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		RX = 0;
	if (abs(RY) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		RY = 0;
	if (RX == 0 && RY == 0)
		return glm::vec2(0, 0);
	//determine how far the controller is pushed
	float magnitude = sqrt(RX * RX + RY * RY);

	//determine the direction the controller is pushed
	float normalizedRX = RX / magnitude;
	float normalizedRY = RY / magnitude;

	return glm::vec2(normalizedRX, normalizedRY);
}


float GamePadXbox::getForceLS()
{
	//copied from DirectX

	float LX = _controllerState.Gamepad.sThumbLX;
	float LY = _controllerState.Gamepad.sThumbLY;

	//determine how far the controller is pushed
	float magnitude = sqrt(LX * LX + LY * LY);

	//determine the direction the controller is pushed
	float normalizedLX = LX / magnitude;
	float normalizedLY = LY / magnitude;

	float normalizedMagnitude = 0;

	//check if the controller is outside a circular dead zone
	if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		//clip the magnitude at its expected maximum value
		if (magnitude > 32767) magnitude = 32767;

		//adjust magnitude relative to the end of the dead zone
		magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

		//optionally normalize the magnitude with respect to its expected range
		//giving a magnitude value of 0.0 to 1.0
		normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	}
	else //if the controller is in the deadzone zero out the magnitude
	{
		magnitude = 0.0f;
		normalizedMagnitude = 0.0f;
	}

	return normalizedMagnitude;
}


float GamePadXbox::getForceRS()
{
	//copied from DirectX

	float RX = _controllerState.Gamepad.sThumbLX;
	float RY = _controllerState.Gamepad.sThumbLY;

	//determine how far the controller is pushed
	float magnitude = sqrt(RX * RX + RY * RY);

	//determine the direction the controller is pushed
	float normalizedRX = RX / magnitude;
	float normalizedRY = RY / magnitude;

	float normalizedMagnitude = 0;

	//check if the controller is outside a circular dead zone
	if (magnitude > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		//clip the magnitude at its expected maximum value
		if (magnitude > 32767) magnitude = 32767;

		//adjust magnitude relative to the end of the dead zone
		magnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

		//optionally normalize the magnitude with respect to its expected range
		//giving a magnitude value of 0.0 to 1.0
		normalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	}
	else //if the controller is in the deadzone zero out the magnitude
	{
		magnitude = 0.0f;
		normalizedMagnitude = 0.0f;
	}

	return normalizedMagnitude;
}

bool GamePadXbox::setControllerNum(GamePadIndex player) {
	// First check range of controller number
	if (player < GamePadIndex_One || player > GamePadIndex_Four)
	{
		return false;
	}

	XINPUT_STATE controllerCurState;
	ZeroMemory(&controllerCurState, sizeof(XINPUT_STATE));

	// Get the state
	DWORD Result = XInputGetState(player, &controllerCurState);

	if (Result == ERROR_SUCCESS)
	{
		this->reset();
		_playerIndex = player;
		_controllerNum = player;
		_controllerState = controllerCurState;
		return true;
	}
	else 
	{
		ZeroMemory(&controllerCurState, sizeof(XINPUT_STATE));
		return false;
	}
}