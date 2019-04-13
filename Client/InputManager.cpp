#include "InputManager.h"

InputManager& InputManager::getInstance() {
  static InputManager inputManager;
  return inputManager;
}

std::unique_ptr<Key> const& InputManager::getKey(int keycode) {
  auto result = _keyList.find(keycode);
  if (result == _keyList.end()) {
    return _keyList.insert({keycode, std::make_unique<Key>(keycode)}).first->second;
  }
  else {
    return result->second;
  }
}

void InputManager::fire(int keycode, KeyState keyState) {
  lock.lock();
  _inputQueue.push_back(std::make_tuple(keycode, keyState));
  lock.unlock();
}

void InputManager::repeat(int keycode) {
  lock.lock();
  _repeatQueue.push_back(std::make_tuple(keycode, KeyState::None));
  lock.unlock();
}

void InputManager::update() {

  std::vector<std::tuple<int, KeyState>> local;
  lock.lock();
  local.insert(local.begin(), _inputQueue.begin(), _inputQueue.end());
  local.insert(local.end(), _repeatQueue.begin(), _repeatQueue.end());
  _inputQueue.clear();
  _repeatQueue.clear();
  lock.unlock();

  for (const auto [keycode, keyState] : local) {
    auto result = _keyList.find(keycode);
    if (result != _keyList.end()) {
      result->second->update(keyState);
    };
  }

}

InputManager::InputManager() {
}
