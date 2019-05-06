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

    nanogui::FormHelper* createFormHelper(const std::string & name);

    nanogui::FormHelper* getFormHelper(const std::string & name);

private:
    bool _dirty = false;

    // nanogui 
    nanogui::Screen * _screen;

    // window
    GLFWwindow* _window = nullptr;

    // Map name to FormHelper
    std::unordered_map<std::string, int> _formHelperMap;

    // List of FormHelper
    std::vector<std::unique_ptr<nanogui::FormHelper>> _formHelpers;

    // Timer
    std::chrono::steady_clock::time_point _lastTime;

    // Test Text Label
    nanogui::Label * _fpsCounter;
};
