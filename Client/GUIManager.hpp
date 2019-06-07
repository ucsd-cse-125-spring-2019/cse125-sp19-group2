#pragma once
#include <nanogui/nanogui.h>
#include <chrono>
#include "GamePadXbox.hpp"
#include "TooltipGUI.hpp"

enum WidgetType {
	WIDGET_CONNECT,
	WIDGET_LOBBY,
	WIDGET_LIST_DOGS,
	WIDGET_LIST_HUMANS,
	WIDGET_LOADING,
	WIDGET_OPTIONS,
	WIDGET_HUD,
	WIDGET_HUD_TOP,
	WIDGET_HUD_MIDDLE,
	WIDGET_HUD_BOTTOM,
	WIDGET_HUD_DOG_SKILLS,
	WIDGET_HUD_HUMAN_SKILLS
};

#define CONNECT_MARGIN 0.15f

// Standard text elements
static const nanogui::Color SOLID_NONE = nanogui::Color(1, 1, 1, 0);
static const nanogui::Color SOLID_HIGHLIGHTED = nanogui::Color(247, 201, 210, 255);
static const nanogui::Color SOLID_WHITE = nanogui::Color(1.0f, 1.0f, 1.0f, 1.0f);
static const nanogui::Color SOLID_GREEN = nanogui::Color(145, 237, 120, 255);

// Skill elements
static const nanogui::Color SKILL_NORMAL = nanogui::Color(0, 160);
static const nanogui::Color SKILL_HIGHLIGHTED = nanogui::Color(168, 255, 255, 160);

// Default font
static const std::string DEFAULT_FONT = "ComicSansBold";

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

	void refresh();

	void resize(int x, int y);

    nanogui::Screen* getScreen();

	// Length of last frame in microseconds
	long getLastFrameLength();

    nanogui::FormHelper* createFormHelper(const std::string & name);

    nanogui::FormHelper* getFormHelper(const std::string & name);

	nanogui::Widget* createWidget(nanogui::Widget* parent, WidgetType name);

	nanogui::Widget* getWidget(WidgetType name);

	// Initializes all widgets
	void initWidgets();

	// Shows the loading screen, given an entity type of dog or human
	void showLoadingScreen(EntityType playerType);

	// Register callbacks for various buttons
	void registerConnectCallback(const std::function<void()> f);
	void registerSwitchSidesCallback(const std::function<void()> f);
	void registerReadyCallback(const std::function<void()> f);
	void registerControllerCallback(const std::function<void(GamePadIndex)> f);
	void registerDisconnectCallback(const std::function<void()> f);

	// Callbacks on mute buttons
	void toggleMute();
	void toggleMusicMute();

	// Toggle FPS
	void toggleFPS();

	// Text in player name and address boxes
	std::string getPlayerName();
	void setPlayerName(std::string val);
	std::string getAddress();
	void setAddress(std::string val);

	// Updates list of dogs/humans in the lobby
	void updateDogsList(
		std::unordered_map<uint32_t, std::string> dogs,
		std::vector<uint32_t> readyPlayers,
		uint32_t playerId);
	void updateHumansList(
		std::unordered_map<uint32_t, std::string> humans,
		std::vector<uint32_t> readyPlayers,
		uint32_t playerId);

	// Sets enabled status for ready and switch buttons
	void setSwitchEnabled(bool enabled);
	void setReadyEnabled(bool enabled);

	// Update clock with milliseconds
	void updateTimer(long milliseconds);

	// Primary and secondary messages for main HUD
	void setPrimaryMessage(std::string message);
	void setSecondaryMessage(std::string message);

	// Set current tooltip for player
	void setTooltip(PlayerTooltip tooltip);

	// Set active skill for human
	void setActiveSkill(bool usePlunger);

	// Update skills with percentages
	void updateStamina(float val);
	void updatePee(float val);
	void updateCharge(float val);

	// Update skills with cooldowns
	void updatePlunger(long millisecondsLeft);
	void updateTrap(long millisecondsLeft);

	// Sets visibility recursively for a given widget
	void setVisibility(WidgetType name, bool visiblity);

	// Hides all widgets
	void hideAll();

private:
	/*** Functions ***/
	void initConnectScreen();
	void initLobbyScreen();
	void initLoadingScreen();
	void initHUD();
	void initControlMenu();

	void setVisibility(nanogui::Widget* widget, bool visibility);

	/*** Variables ***/

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

	// Base font scale
	int _baseFontScale = 1280;

	// Scaling stuff
	int _largeFontSize = 90;
	int _mediumFontSize = 68;
	int _smallFontSize = 45;

	int _scaledSmallFontSize = _smallFontSize;
	int _scaledMediumFontSize = _mediumFontSize;
	int _scaledLargeFontSize = _largeFontSize;

	int _baseCooldownSize = 100;
	int _baseCooldownOffset = 6;
	int _baseButtonWidth = 100;
	int _baseButtonHeight = 40;

	// Connect screen
	nanogui::Label* _connectPadding;
	nanogui::TextBox* _playerNameBox;
	nanogui::TextBox* _addressBox;
	nanogui::Button* _connectButton;

	// Lobby screen
	nanogui::Button* _optionsButton;
	nanogui::Button* _switchSidesButton;
	nanogui::Label* _lobbyPadding;
	nanogui::Button* _readyButton;

	// Loading screen
	GLuint _dogsLoadingScreen;
	GLuint _humansLoadingScreen;

	// Controls menu
	nanogui::detail::FormWidget<GamePadIndex, std::integral_constant<bool, true>>* _gamepadSelect;
	nanogui::Button* _muteButton;
	nanogui::Button* _muteMusicButton;
	nanogui::Button* _disconnectButton;

	// Upper HUD
	nanogui::Label* _timer;	// Countdown timer

	// Middle HUD
	nanogui::Label* _primaryMessage;	// Game over, etc.
	nanogui::Label* _secondaryMessage;	// Time to start, etc.

	// Lower HUD

	// Dog skills
	nanogui::Label* _staminaInfo;	// Not used
    nanogui::CooldownBar* _staminaCooldown;
	nanogui::Label* _peeInfo;	// Not used
	nanogui::CooldownBar* _peeCooldown;

	// Human skills
	nanogui::Label* _plungerInfo;
	nanogui::CooldownBar* _plungerCooldown;
	nanogui::Label* _trapInfo;
	nanogui::CooldownBar* _trapCooldown;
	nanogui::Label* _chargeInfo;
	nanogui::CooldownBar* _swingCharge;
	GLuint _plungerIcon;
	GLuint _plungerIconSelected;
	GLuint _trapIcon;
	GLuint _trapIconSelected;
	GLuint _swingIcon;
	GLuint _swingIconSelected;

	// Tooltips
	std::unique_ptr<TooltipGUI> _tooltipGUI;
};
