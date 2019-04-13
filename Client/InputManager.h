#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include <unordered_map>
#include "Key.h"
#include <queue>
#include <iterator>
#include <mutex>

class InputManager {
public:
  static InputManager& getInstance();

  /**
   * \brief 
   * Usage: Register(GLFW_KEY_G)->OnPress([]{
   *   Something ...
   * });
   * \param scancode GLFW Keycode
   */
  std::unique_ptr<Key> const& getKey(int keycode);

  void fire(int keycode, KeyState keyState);

  void repeat(int keycode);

  void update();

private:
  InputManager();;

  std::unordered_map<int, std::unique_ptr<Key>> _keyList;
  std::vector<std::tuple<int, KeyState>> _inputQueue;
  std::vector<std::tuple<int, KeyState>> _repeatQueue;
  std::mutex lock;
};

#endif
