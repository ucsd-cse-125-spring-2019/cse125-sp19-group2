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

Key2D::Key2D(int keyType): Key(keyType) {

}

void Key2D::update(glm::vec2 pos) {
    // if it's mouse, use delta
    if (_keycode == KEYTYPE::MOUSE) {

        // Calculate mouse delta
        const auto curr = pos;
        _lastDelta = curr - _lastPos;

        // Call events if the delta is larger than something
        if (_lastDelta.length() > 0.001) {
            for (auto f : _onMove) {
                f(_lastDelta);
            }
        }

        _lastPos = pos;
    }else {
        // else use pos directly
        // Call events if the delta is larger than something
        if (pos.length() > 0.001) {
            for (auto f : _onMove) {
                f(pos);
            }
        }
    }

}
