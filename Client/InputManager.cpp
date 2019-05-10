#include "InputManager.h"

InputManager& InputManager::getInstance() {
    static InputManager inputManager;
    return inputManager;
}

std::unique_ptr<Key> const& InputManager::getKey(int keycode) {
    auto result = _keyList.find(keycode);
    if (result == _keyList.end()) {
        if(keycode >= 0){
            return _keyList.insert({keycode, std::make_unique<Key>(keycode)}).first->second;
		}else {
		    return _keyList.insert({keycode, std::make_unique<Key2D>(keycode)}).first->second;
		}
    }
    else {
        return result->second;
    }
}

void InputManager::fire(int keycode, KeyState keyState) {
    _lock.lock();
    _inputQueue.push_back(std::make_tuple(keycode, keyState));
    _lock.unlock();
}

void InputManager::repeat(int keycode) {
    _lock.lock();
    _repeatQueue.push_back(std::make_tuple(keycode, KeyState::None));
    _lock.unlock();
}

void InputManager::move(int keyType, double x, double y) {
    _lock.lock();
    _moveQueue.push_back(std::make_tuple(keyType, glm::vec2(x, y)));
    _lock.unlock();
}

void InputManager::scroll(double s) {
    // Call onScroll functions if the scroll is larger than something
    if (abs(s) > 0.001) {
        for (auto f : _onScroll) {
            f(s);
        }
    }
}

void InputManager::update() {

    std::vector<std::tuple<int, KeyState>> local;
    std::vector<std::tuple<int, glm::vec2>> localMove;
    _lock.lock();
    local.insert(local.begin(), _inputQueue.begin(), _inputQueue.end());
    local.insert(local.end(), _repeatQueue.begin(), _repeatQueue.end());
    localMove.insert(localMove.end(), _moveQueue.begin(), _moveQueue.end());
    _inputQueue.clear();
    _repeatQueue.clear();
    _moveQueue.clear();
    _lock.unlock();

    for (const auto& [keycode, keyPos] : localMove) {
        auto result = _keyList.find(keycode);
        if (result != _keyList.end()) {
            static_cast<Key2D*>(result->second.get())->update(keyPos);
        };
    }

    for (const auto& [keycode, keyState] : local) {
        auto result = _keyList.find(keycode);
        if (result != _keyList.end()) {
            result->second->update(keyState);
        };
    }
}

InputManager::InputManager() {

}
