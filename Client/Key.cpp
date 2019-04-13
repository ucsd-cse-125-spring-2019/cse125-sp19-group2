#include "Key.h"
#include "InputManager.h"

Key::Key(int keycode) {
  _keycode = keycode;
}

Key::~Key() {

}

void Key::update(KeyState const keyState) {

  // on release
  if ((keyState & KeyState::Release)) {

    for (auto f : _onRelease) {
      f();
    }
    _keyState = keyState;
    _keyState &= ~KeyState::Repeat;
  }

  // on repeat
  if (_keyState & KeyState::Repeat) {

    for (auto f : _onRepeat) {
      f();
    }

    // send None event to repeat
    repeatInput();
  }

  // on press
  if ((keyState & KeyState::Press) && !(_keyState & KeyState::Press)) {
    for (auto f : _onPress) {
      f();
    }
    _keyState = keyState;
    _keyState |= KeyState::Repeat;

    // send None event to repeat
    repeatInput();
  }

}

void Key::repeatInput() {
  InputManager::getInstance().repeat(_keycode);
}
