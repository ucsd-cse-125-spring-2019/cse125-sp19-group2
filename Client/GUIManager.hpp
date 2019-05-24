#pragma once
#include <nanogui/nanogui.h>
#include <chrono>
#include "GamePadXbox.hpp"

enum WidgetType {
	WIDGET_CONNECT,
	WIDGET_LOBBY,
	WIDGET_LIST_DOGS,
	WIDGET_LIST_HUMANS,
	WIDGET_OPTIONS,
	WIDGET_HUD,
	WIDGET_HUD_TOP,
	WIDGET_HUD_MIDDLE,
	WIDGET_HUD_BOTTOM
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

	void redraw(nanogui::Widget* widget);

	void resize(int x, int y);

    nanogui::Screen* getScreen();

    void setDirty();

	// Length of last frame in microseconds
	long getLastFrameLength();

    nanogui::FormHelper* createFormHelper(const std::string & name);

    nanogui::FormHelper* getFormHelper(const std::string & name);

	nanogui::Widget* createWidget(nanogui::Widget* parent, WidgetType name);

	nanogui::Widget* getWidget(WidgetType name);

	// Initializes all widgets
	void initWidgets();

	// Register callbacks for various buttons
	void registerConnectCallback(const std::function<void()> f);
	void registerSwitchSidesCallback(const std::function<void()> f);
	void registerReadyCallback(const std::function<void()> f);
	void registerControllerCallback(const std::function<void(GamePadIndex)> f);

	// Returns text in player name and address boxes
	std::string getPlayerName();
	std::string getAddress();

	// Updates list of dogs/humans in the lobby
	void updateDogsList(std::unordered_map<uint32_t, std::string> dogs, uint32_t playerId);
	void updateHumansList(std::unordered_map<uint32_t, std::string> humans, uint32_t playerId);

	// Sets text for ready button
	void setReadyText(std::string text);

	// Sets enabled status for ready and switch buttons
	void setSwitchEnabled(bool enabled);
	void setReadyEnabled(bool enabled);

	// Sets visibility recursively for a given widget
	void setVisibility(WidgetType name, bool visiblity);

	// Hides all widgets
	void hideAll();

private:
	/*** Functions ***/
	void initConnectScreen();
	void initLobbyScreen();
	void initHUD();
	void initControlMenu();

	void setVisibility(nanogui::Widget* widget, bool visibility);

	/*** Variables ***/
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

	// Time since last frame in microseconds
	long _lastFrameLength;

    // Test Text Label
    nanogui::Label * _fpsCounter;

	// Lists of dogs and humans; maps IDs to player names
	std::unordered_map<std::string, std::string> _dogs;
	std::unordered_map<std::string, std::string> _humans;

	/*** UI elements ***/

	// Connect screen
	nanogui::TextBox* _playerNameBox;
	nanogui::TextBox* _addressBox;
	nanogui::Button* _connectButton;

	// Lobby screen
	nanogui::Button* _switchSidesButton;
	nanogui::Button* _readyButton;

	// Controls menu
	nanogui::detail::FormWidget<GamePadIndex, std::integral_constant<bool, true>>* _gamepadSelect;

	// Upper HUD

	// Middle HUD

	// Lower HUD
};
