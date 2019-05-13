#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include <unordered_map>
#include "Key.h"
#include <queue>
#include <iterator>
#include <mutex>
#include <glm/vec2.hpp>
#include <GLFW/glfw3.h>

/**
 * \brief A singleton class that receives and processes keyboard inputs
 */
class InputManager {
public:

    /**
     * \brief The singleton getter of InputManager (create one if not exist)
     * \return InputManager&: An InputManager Object
     */
    static InputManager& getInstance();

    /**
     * \brief Get the reference of the Key object
     * \verbatim
     * Usage: getKey(GLFW_KEY_G)->onPress([&]{
     *   do something ...
     * });
     * \endverbatim
     * \param keycode(int) GLFW Keycode
     * \return std::unique_ptr<Key> const&: Key reference to the corresponding keycode.
     */
    Key* getKey(int keycode);

    Key2D* getKey2D(int keycode);

    /**
     * \brief Send the input-related event to InputManager's queue
     * \param keycode(int) GLFW Keycode
     * \param keyState(KeyState) KeyState of the event [KeyState::Press | KeyState::Release]
     */
    void fire(int keycode, KeyState keyState);

    /**
     * \brief Send the input-related event to InputManager's queue for repeat event
     * \param keycode(int) keycode GLFW Keycode
     */
    void repeat(int keycode);

    void move(int keyType, double x, double y);

    void scroll(double s);

    /**
     * \brief Process all input events inside the queue
     */
    void update();

    void setWindow(GLFWwindow* window);

    GLFWwindow* getWindow() const;

    bool isForegroundWindow() const;

    template <typename T>
    void onScroll(T&& lambda);

private:
    InputManager();

    std::unordered_map<int, std::unique_ptr<Key>> _keyList;
    std::vector<std::tuple<int, KeyState>> _inputQueue;
    std::vector<std::tuple<int, KeyState>> _repeatQueue;
    std::vector<std::tuple<int, glm::vec2>> _moveQueue;
    std::vector<std::function<void(float)>> _onScroll;

    std::mutex _lock;
    GLFWwindow* _window = nullptr;
};

template <typename T>
void InputManager::onScroll(T&& lambda) {
    _onScroll.push_back(lambda);
}

#endif
