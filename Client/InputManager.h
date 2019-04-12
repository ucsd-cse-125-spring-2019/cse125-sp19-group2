#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include <unordered_map>
#include "Key.h"
#include <queue>
#include <iterator>
#include <mutex>

class InputManager {
public:
  static InputManager& GetInstance();

  /**
   * \brief 
   * Usage: Register(GLFW_KEY_G)->OnPress([]{
   *   Something ...
   * });
   * \param scancode GLFW Keycode
   */
  std::unique_ptr<Key> const& Register(int keycode);

  void Fire(int keycode, KeyState keyState);

  void Repeat(int keycode);

  void Update();

private:
  InputManager();;

  std::unordered_map<int, std::unique_ptr<Key>> _keyList;
  std::vector<std::tuple<int, KeyState>> _inputQueue;
  std::vector<std::tuple<int, KeyState>> _repeatQueue;
  std::mutex lock;
};

#endif
