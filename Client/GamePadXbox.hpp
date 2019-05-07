#include <windows.h>
#include <XInput.h>     // XInput API
#include "../Shared/Logger.hpp"
#pragma comment(lib, "XInput.lib")   // Library. If your compiler doesn't support this type of lib include change to the corresponding one

class Vector2
{
private:
	float x;
	float y;
public:
	Vector2(float x, float y) {
		this->x = x;
		this->y = y;
	}
	float getX() {
		return this->x;
	}
	float getY() {
		return this->y;
	}
};

typedef enum
{

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

class GamePadXbox
{
public:
	GamePadXbox(GamePadIndex player)
	{
		_playerIndex = player;
		_controllerNum = 0;
		reset();
	}

	virtual ~GamePadXbox(void)
	{
		// We don't want the controller to be vibrating accidentally when we exit the app
		if (isConnected()) setVibration(Vector2(0.0f, 0.0f));
	}

	bool isConnected();
	void reset();
	bool isKeyUp(int button);
	bool isKeyDown(int button);
	bool getKeyState(int button);
	//LT, RT press level: [0,1]
	float getForceLT();
	float getForceRT();
	//LS, RS move vector: [0,1]
	Vector2 getLS();
	Vector2 getRS();
	//LS, RS from the origin: [0,1]
	float getForceLS();
	float getForceRS();
	// Set Vibration: [0,1]
	void setVibration(Vector2 lr);
private:
	XINPUT_STATE _controllerState;
	XINPUT_STATE _controllerStatePre;
	GamePadIndex _playerIndex;
	int _controllerNum;
};