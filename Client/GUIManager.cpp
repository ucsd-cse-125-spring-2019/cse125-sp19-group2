#include "GuiManager.hpp"
#include <chrono>

using namespace nanogui;
using namespace std;
using namespace chrono;

GuiManager& GuiManager::getInstance() {
    static GuiManager guiManager;
    return guiManager;
}

GuiManager::GuiManager() {
    // Nanogui init
    _screen = new Screen();
}

void GuiManager::init(GLFWwindow* window) {

    _window = window;
    _screen->initialize(_window, false);
    _screen->setVisible(true);
    _screen->shutdownGLFWOnDestruct();

    glfwSetCharCallback(
        _window, [](GLFWwindow*, unsigned int codepoint) {
            GuiManager::getInstance().getScreen()->charCallbackEvent(codepoint);
        });
    glfwSetDropCallback(
        _window, [](GLFWwindow*, int count, const char** filenames) {
            GuiManager::getInstance().getScreen()->dropCallbackEvent(count, filenames);
        });

    _fpsCounter = new Label(_screen, "fps: 0", "sans", 32);
    _fpsCounter->setColor(Color(Vector4f(1,1,1,1)));
    
}

void GuiManager::draw() {

    if(_lastTime.time_since_epoch().count() == 0) {
        _lastTime = high_resolution_clock::now();
    }else {
        auto curr = high_resolution_clock::now();
        auto dt = curr - _lastTime;
        auto fps = 1000000.0  / chrono::duration_cast<microseconds>(dt).count();
        _fpsCounter->setCaption("fps:" + std::to_string(int(fps)));
        _fpsCounter->setSize(_fpsCounter->preferredSize(_screen->nvgContext()));
        _lastTime = high_resolution_clock::now();
    }

    if (_dirty) {
        // Recalculate widget
        _screen->performLayout();
        _dirty = false;
    }

    for (auto & element : _formHelpers) {
        element->refresh();
    }

    // Draw nanogui
    _screen->drawContents();
    _screen->drawWidgets();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

Screen* GuiManager::getScreen() {
    return _screen;
}

void GuiManager::setDirty() {
    _dirty = true;
}

FormHelper* GuiManager::createFormHelper(const string & name) {
    auto f = make_unique<FormHelper>(_screen);
    _formHelpers.push_back(std::move(f));
    _formHelperMap.insert({name, _formHelpers.size() - 1});
    return _formHelpers.back().get();
}

nanogui::FormHelper* GuiManager::getFormHelper(const std::string& name) {
    const auto res = _formHelperMap.find(name);
    if(res != _formHelperMap.end()) {
        return _formHelpers[res->second].get();
    }
    return nullptr;
}
