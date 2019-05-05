#pragma once
#include <nanogui/nanogui.h>
#include <chrono>

class GuiManager {
public: 
    /**
     * \brief The singleton getter of GuiManager (create one if not exist)
     * \return GuiManager&: An GuiManager Object
     */
    static GuiManager& getInstance();

    GuiManager();

    void init(GLFWwindow* window);

    void draw();

    nanogui::Screen* getScreen();

    void setDirty();

    nanogui::FormHelper* createFormHelper();

private:
    bool _dirty = false;

    // nanogui 
    nanogui::Screen * _screen;

    // window
    GLFWwindow* _window = nullptr;

    // List of FormHelper
    std::vector<std::unique_ptr<nanogui::FormHelper>> formHelpers;

    // Timer
    std::chrono::steady_clock::time_point _lastTime;

    // Test Text Label
    nanogui::Label * _fpsCounter;
};
