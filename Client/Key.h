#ifndef KEY_H
#define KEY_H

#include <vector>
#include <any>
#include <functional>
#include <Windows.h>

enum KeyState : uint32_t {
  None = 0,
  Press = (1 << 0),
  Release = (1 << 1),
  Repeat = (1 << 2),
  // Not Used
  Shift = (1 << 3),
  Ctrl = (1 << 4),
  Alt = (1 << 5)
};

DEFINE_ENUM_FLAG_OPERATORS(KeyState)

class Key {
public:

  Key(int keycode);

  ~Key();

  template <typename T>
  void OnPress(T&& lambda);

  template <typename T>
  void OnRelease(T&& lambda);

  template <typename T>
  void OnRepeat(T&& lambda);

  void Update(KeyState const keyState);

private:
  void RepeatInput();

  int _keycode;
  KeyState _keyState;

  std::vector<std::function<void()>> _onPress;
  std::vector<std::function<void()>> _onRelease;
  std::vector<std::function<void()>> _onRepeat;

};

template <typename T>
void Key::OnPress(T&& lambda) {
	_onPress.push_back(lambda);
}

template <typename T>
void Key::OnRelease(T&& lambda) {
	_onRelease.push_back(lambda);
}

template <typename T>
void Key::OnRepeat(T&& lambda) {
	_onRepeat.push_back(lambda);
}

#endif
