#pragma once
#include <nanogui/nanogui.h>
#include <chrono>

enum WidgetType {
	WIDGET_CONNECT,
	WIDGET_LOBBY,
	WIDGET_LIST_DOGS,
	WIDGET_LIST_HUMANS,
	WIDGET_OPTIONS,
	WIDGET_HUD
};

#define CONNECT_MARGIN 0.15f

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

	void resize(int x, int y);

    nanogui::Screen* getScreen();

    void setDirty();

    nanogui::FormHelper* createFormHelper(const std::string & name);

    nanogui::FormHelper* getFormHelper(const std::string & name);

	nanogui::Widget* createWidget(WidgetType name);

	nanogui::Widget* getWidget(WidgetType name);

	// Hides all widgets
	void hideAll();

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

	// Map window name to window
	std::unordered_map<WidgetType, nanogui::Widget*> _widgets;

    // Timer
    std::chrono::steady_clock::time_point _lastTime;

    // Test Text Label
    nanogui::Label * _fpsCounter;
};
