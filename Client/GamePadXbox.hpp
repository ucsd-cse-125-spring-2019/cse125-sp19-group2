#pragma once
#define WIN32_LEAN_AND_MEAN // We don't want the extra stuff like MFC and such
#include <windows.h>
#include <XInput.h>     // XInput API
#include "../Shared/Logger.hpp"
#pragma comment(lib, "XInput.lib")   // Library. If your compiler doesn't support this type of lib include change to the corresponding one
typedef enum
{
	//GamePad_Button_DPAD_UP = 0,
	//GamePad_Button_DPAD_DOWN = 1,
	//GamePad_Button_DPAD_LEFT = 2,
	//GamePad_Button_DPAD_RIGHT = 3,
	//GamePad_Button_START = 4,
	//GamePad_Button_BACK = 5,
	//GamePad_Button_LEFT_THUMB = 6,
	//GamePad_Button_RIGHT_THUMB = 7,
	//GamePad_Button_LEFT_SHOULDER = 8,
	//GamePad_Button_RIGHT_SHOULDER = 9,
	//GamePad_Button_A = 10,
	//GamePad_Button_B = 11,
	//GamePad_Button_X = 12,
	//GamePad_Button_Y = 13,

	GamePad_Button_DPAD_UP = 0x00000001,
	GamePad_Button_DPAD_DOWN = 0x00000002,
	GamePad_Button_DPAD_LEFT = 0x00000004,
	GamePad_Button_DPAD_RIGHT = 0x00000008,
	GamePad_Button_START = 0x00000010,
	GamePad_Button_BACK = 0x00000020,
	GamePad_Button_LEFT_SHOULDER = 0x00000040,
	GamePad_Button_RIGHT_SHOULDER = 0x00000080,
	GamePad_Button_LEFT_THUMB = 0x0100,
	GamePad_Button_RIGHT_THUMB = 0x0200,
	GamePad_Button_A = 0x1000,
	GamePad_Button_B = 0x2000,
	GamePad_Button_X = 0x4000,
	GamePad_Button_Y = 0x8000,
	GamePadButton_Max = 14,


}GamePadButton;
// GamePad Indexes
typedef enum
{
	GamePadIndex_One = 0,
	GamePadIndex_Two = 1,
	GamePadIndex_Three = 2,
	GamePadIndex_Four = 3,
}GamePadIndex;
// The GamePad State Stuct, were we store the buttons positions
struct GamePadState
{
	bool		_buttons[GamePadButton_Max];
	float 		_left_thumbstick;
	float	    _right_thumbstick;
	float		_left_trigger;
	float		_right_trigger;
	// Just to clear all values to default
	void reset()
	{
		for (int i = 0; i < (int)GamePadButton_Max; ++i) _buttons[i] = false;
		_left_thumbstick = 0.0f;
		_right_thumbstick = 0.0f;
		_left_trigger = _right_trigger = 0.0f;
	}
};
class GamePadXbox
{
public:
	GamePadXbox(GamePadIndex player)
	{
		_playerIndex = player;
		State.reset();
		_controllerNum = 0;

	}

	virtual ~GamePadXbox(void)
	{
		// We don't want the controller to be vibrating accidentally when we exit the app
		if (is_connected()) vibrate(0.0f, 0.0f);
	}

	bool is_connected();
	void vibrate(float, float);
	void reset();
	bool KeyUp(int button);
	bool KeyDown(int button);
	bool KeyState(int button);
public:
	GamePadState	State;
private:
	XINPUT_STATE _controllerState;
	XINPUT_STATE _controllerStatePre;
	GamePadIndex _playerIndex;
	int _controllerNum;
};