#include "GamePadXbox.hpp"

bool GamePadXbox::is_connected()
{
	_controllerStatePre = _controllerState;

	// clean the state
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

	// Get the state
	DWORD Result = XInputGetState(_controllerNum, &_controllerState);

	if (Result == ERROR_SUCCESS)	return true;
	else return false;
}
void GamePadXbox::vibrate(float leftmotor = 0.0f, float rightmotor = 0.0f)
{
	// Create a new Vibraton 
	XINPUT_VIBRATION vibration;

	memset(&vibration, 0, sizeof(XINPUT_VIBRATION));

	int leftVib = (int)(leftmotor * 65535.0f);
	int rightVib = (int)(rightmotor * 65535.0f);

	// Set the Vibration Values
	vibration.wLeftMotorSpeed = leftVib;
	vibration.wRightMotorSpeed = rightVib;
	// Vibrate the controller
	XInputSetState((int)_controllerNum, &vibration);

}
void GamePadXbox::reset()
{
	State.reset();
}

bool GamePadXbox::KeyUp(int button)
{
	return !(_controllerState.Gamepad.wButtons & button) &&
		(_controllerStatePre.Gamepad.wButtons & button);
}

bool GamePadXbox::KeyDown(int button)
{
	return (_controllerState.Gamepad.wButtons & button) &&
		!(_controllerStatePre.Gamepad.wButtons & button);
}

bool GamePadXbox::KeyState(int button)
{
	return _controllerState.Gamepad.wButtons & button;
}