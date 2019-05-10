#ifndef KEY_H
#define KEY_H

#include <vector>
#include <any>
#include <functional>
#include <Windows.h>
#include <glm/vec2.hpp>
#include <glm/gtx/norm.inl>

/**
 * \brief bitflag enum for keystate
 */
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

/**
 * \brief Class to manage keystate and dispatch functions for a corresponding key
 */
class Key {
public:

    enum KEYTYPE {
        MOUSE = -1,
        LSTICK = -2,
        RSTICK = -3
    };

    /**
     * \brief Construct a Key object and set its keycode.
     * \param keycode(int) GLFW keycode
     */
    Key(int keycode);

    ~Key();

    /**
     * \brief Register a function to this key
     * \tparam T
     * \param lambda(T&&) A function will be invoked when press event happens
     */
    template <typename T>
    void onPress(T&& lambda);

    /**
     * \brief Register a function to this key
     * \tparam T
     * \param lambda(T&&) A function will be invoked when release event happens
     */
    template <typename T>
    void onRelease(T&& lambda);

    /**
     * \brief Register a function to this key
     * \tparam T 
     * \param lambda(T&&) A function will be invoked when repeat event happens
     */
    template <typename T>
    void onRepeat(T&& lambda);

    /**
     * \brief Process an input corresponding to this key and invoke related functions.
     * \param keyState(KeyState const) KeyState of the event [KeyState::Press | KeyState::Release]
     */
    void update(KeyState const keyState);

protected:
    void repeatInput();

    int _keycode;
    KeyState _keyState;

private:
    std::vector<std::function<void()>> _onPress;
    std::vector<std::function<void()>> _onRelease;
    std::vector<std::function<void()>> _onRepeat;

};

class Key2D : public Key {
public:

    Key2D(int keyType);

    void update(glm::vec2 pos);

    template <typename T>
    void onMove(T&& lambda);

private:
    std::vector<std::function<void(glm::vec2)>> _onMove;
    glm::vec2 _lastDelta;
    glm::vec2 _lastPos;
};

template <typename T>
void Key::onPress(T&& lambda) {
    _onPress.push_back(lambda);
}

template <typename T>
void Key::onRelease(T&& lambda) {
    _onRelease.push_back(lambda);
}

template <typename T>
void Key::onRepeat(T&& lambda) {
    _onRepeat.push_back(lambda);
}

template <typename T>
void Key2D::onMove(T&& lambda) {
    _onMove.push_back(lambda);
}

#endif
