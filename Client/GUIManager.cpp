#include "GuiManager.hpp"
#include "Shared/Logger.hpp"
#include "stb_image.h"
#include <chrono>
#include "Texture.hpp"

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
    _fpsCounter->setColor(SOLID_WHITE);
    
}

void GuiManager::draw() {

    if(_lastTime.time_since_epoch().count() == 0) {
        _lastTime = high_resolution_clock::now();
    }else {
        auto curr = high_resolution_clock::now();
        auto dt = curr - _lastTime;
		_lastFrameLength = std::chrono::duration_cast<microseconds>(dt).count();
		auto fps = 1000000.0 / _lastFrameLength;
        _fpsCounter->setCaption("fps:" + std::to_string(int(fps)));
        _fpsCounter->setSize(_fpsCounter->preferredSize(_screen->nvgContext()));
        _lastTime = high_resolution_clock::now();
    }

	// Recompute layout
    _screen->performLayout();

    for (auto & element : _formHelpers) {
        element->refresh();
    }

    // Draw nanogui
    _screen->drawContents();
    _screen->drawWidgets();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

void GuiManager::resize(int x, int y) {
	_screen->resizeCallbackEvent(x, y);

	// Resize our widgets
	for (auto& widgetPair : _widgets) {
		if (widgetPair.first == WIDGET_OPTIONS) {
			// Options menu
			widgetPair.second->setFixedSize(nanogui::Vector2i(x / _screen->pixelRatio() / 3, y / _screen->pixelRatio() / 1.5));
		}
		else if (widgetPair.first == WIDGET_CONNECT ||
			widgetPair.first == WIDGET_LOBBY ||
			widgetPair.first == WIDGET_HUD) {
			widgetPair.second->setFixedSize(nanogui::Vector2i(x / _screen->pixelRatio(), y / _screen->pixelRatio()));
		}

		// Resize layout margins
		switch (widgetPair.first) {
		case WIDGET_CONNECT:
			static_cast<nanogui::BoxLayout*>(widgetPair.second->layout())->setMargin(x * CONNECT_MARGIN);
			break;
		case WIDGET_HUD:
			// Set spacing for parent HUD widget to "clamp" bottom and top HUD 
			int topHeight = getWidget(WIDGET_HUD_TOP)->preferredSize(_screen->nvgContext()).y() / _screen->pixelRatio();
			int middleHeight = getWidget(WIDGET_HUD_MIDDLE)->preferredSize(_screen->nvgContext()).y() / _screen->pixelRatio();
			int bottomHeight = getWidget(WIDGET_HUD_BOTTOM)->preferredSize(_screen->nvgContext()).y() / _screen->pixelRatio();

			int spacing = (y / _screen->pixelRatio() - middleHeight - topHeight - bottomHeight);
			static_cast<nanogui::BoxLayout*>(widgetPair.second->layout())->setSpacing(spacing / 2);
			break;
		}
	}
}

Screen* GuiManager::getScreen() {
    return _screen;
}

long GuiManager::getLastFrameLength()
{
	return _lastFrameLength;
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

nanogui::Widget* GuiManager::createWidget(nanogui::Widget* parent, WidgetType name) {
	nanogui::Widget* widget;

	if (name == WIDGET_OPTIONS) {
		widget = new nanogui::Window(parent, "Options");
	}
	else {
		widget = new nanogui::Widget(parent);
	}

	_widgets.insert({ name, widget });
	return widget;
}

nanogui::Widget* GuiManager::getWidget(WidgetType name) {
	const auto res = _widgets.find(name);
	if (res != _widgets.end()) {
		return res->second;
	}
	return nullptr;
}

void GuiManager::initWidgets() {
	initConnectScreen();
	initLobbyScreen();
	initHUD();
	initControlMenu();
	hideAll();
}

void GuiManager::registerConnectCallback(const std::function<void()> f) {
	_connectButton->setCallback(f);
}

void GuiManager::registerSwitchSidesCallback(const std::function<void()> f) {
	_switchSidesButton->setCallback(f);
}

void GuiManager::registerReadyCallback(const std::function<void()> f) {
	_readyButton->setCallback(f);
}

void GuiManager::registerControllerCallback(const std::function<void(GamePadIndex)> f)
{
	_gamepadSelect->setCallback(f);
}

std::string GuiManager::getPlayerName() {
	return _playerNameBox->value();
}

void GuiManager::setPlayerName(std::string val) {
	_playerNameBox->setValue(val);
}

std::string GuiManager::getAddress() {
	return _addressBox->value();
}

void GuiManager::setAddress(std::string val) {
	_addressBox->setValue(val);
}

void GuiManager::updateDogsList(
	std::unordered_map<uint32_t, std::string> dogs,
	uint32_t playerId) {

	auto dogList = GuiManager::getInstance().getWidget(WIDGET_LIST_DOGS);

	// Create a list for widgets that need to be removed
	std::vector<nanogui::Widget*> removeList;

	// Find Already Left Dog Players
	for (auto it = dogList->children().begin(); it != dogList->children().end(); ++it)
	{
		uint32_t id = atoi((*it)->id().c_str());
		// if this id can not be found, save its widget to removeList
		if (dogs.find(id) == dogs.end())
		{
			removeList.push_back(*it);
		}
	}
	// Remove all widges in the removeList
	for (auto it = removeList.begin(); it != removeList.end(); ++it) {
		_dogs.erase((*it)->id());
		dogList->removeChild(*it);
	}
	removeList.clear();

	// Add dogs
	for (auto& dogPair : dogs)
	{
		auto dogId = std::to_string(dogPair.first);
		// if current dog is not in our list, add it
		if (_dogs.find(dogId) == _dogs.end())
		{
			auto dogName = dogPair.second;
			auto playerLabel = new nanogui::Label(dogList, dogName, "sans", 28);
			playerLabel->setId(dogId);
			_dogs[dogId] = dogName;
			if (dogPair.first == playerId)
			{
				playerLabel->setColor(SOLID_HIGHLIGHTED);
			}
		}
	}
}

void GuiManager::updateHumansList(
	std::unordered_map<uint32_t, std::string> humans,
	uint32_t playerId) {

	auto humanList = getWidget(WIDGET_LIST_HUMANS);

	// Create a list for widgets that need to be removed
	std::vector<nanogui::Widget*> removeList;

	// Find Already Left Human Players
	for (auto it = humanList->children().begin(); it != humanList->children().end(); ++it)
	{
		uint32_t id = atoi((*it)->id().c_str());
		// if this id can not be found, save its widget to removeList
		if (humans.find(id) == humans.end())
		{
			removeList.push_back(*it);
		}
	}
	// Remove all widges in the removeList
	for (auto it = removeList.begin(); it != removeList.end(); ++it) {
		_humans.erase((*it)->id());
		humanList->removeChild(*it);
	}
	removeList.clear();

	// Add humans
	for (auto& humanPair : humans)
	{
		auto humanId = std::to_string(humanPair.first);
		// if current dog is not in our list, add it
		if (_humans.find(humanId) == _humans.end())
		{
			auto humanName = humanPair.second;
			auto playerLabel = new nanogui::Label(humanList, humanPair.second, "sans", 28);
			playerLabel->setId(humanId);
			_humans[humanId] = humanName;
			if (humanPair.first == playerId)
			{
				playerLabel->setColor(SOLID_HIGHLIGHTED);
			}
		}
	}
}

void GuiManager::setReadyText(std::string text) {
	_readyButton->setCaption(text);
}

void GuiManager::setSwitchEnabled(bool enabled) {
	_switchSidesButton->setEnabled(enabled);
}

void GuiManager::setReadyEnabled(bool enabled) {
	_readyButton->setEnabled(enabled);
}

void GuiManager::updateTimer(long milliseconds) {
	// Format milliseconds first
	long seconds = (milliseconds / 1000) % 60;
	long minutes = (milliseconds / (1000 * 60)) % 60;

	// String buffer
	char buf[10];
	snprintf(buf, 10, "%02d:%02d.%03d", minutes, seconds, milliseconds % 1000);

	_timer->setCaption(std::string(buf));
}

void GuiManager::setPrimaryMessage(std::string message) {
	_primaryMessage->setCaption(message);
}

void GuiManager::setSecondaryMessage(std::string message) {
	_secondaryMessage->setCaption(message);
}

void GuiManager::setActiveSkill(bool usePlunger) {
	if (usePlunger) {
		_plungerInfo->setColor(SOLID_HIGHLIGHTED);
		_trapInfo->setColor(SOLID_WHITE);
	}
	else
	{
		_plungerInfo->setColor(SOLID_WHITE);
		_trapInfo->setColor(SOLID_HIGHLIGHTED);
	}
}

void GuiManager::updateStamina(float val) {
	int percentage = (int)(val * 100);
	_staminaInfo->setCaption("Stamina: " + std::to_string(percentage) + "%");
}

void GuiManager::updatePee(float val) {
	int percentage = (int)(val * 100);
	_peeInfo->setCaption("Pee: " + std::to_string(percentage) + "%");
}

void GuiManager::updatePlunger(long millisecondsLeft) {
	if (millisecondsLeft == 0)
	{
		_plungerInfo->setCaption("Plunger: Ready");
	}
	else
	{
		auto seconds = (int)(std::ceil(millisecondsLeft / 1000.0f));
		_plungerInfo->setCaption("Plunger: " + std::to_string(seconds));
	}
}

void GuiManager::updateTrap(long millisecondsLeft) {
	if (millisecondsLeft == 0)
	{
		_trapInfo->setCaption("Trap: Ready");
	}
	else
	{
		auto seconds = (int)(std::ceil(millisecondsLeft / 1000.0f));
		_trapInfo->setCaption("Trap: " + std::to_string(seconds));
	}
}

void GuiManager::updateCharge(float val) {
	int percentage = (int)(val * 100);
	_chargeInfo->setCaption("Charge: " + std::to_string(percentage) + "%");
}

void GuiManager::setVisibility(WidgetType name, bool visibility) {
	setVisibility(_widgets.find(name)->second, visibility);
}

void GuiManager::hideAll() {
	for (auto& widgetPair : _widgets) {
		widgetPair.second->setVisible(false);
	}
}

/*** Private functions ***/
void GuiManager::initConnectScreen() {
	auto connectScreen = createWidget(_screen, WIDGET_CONNECT);
    auto bg = LoadTextureFromFile("background.png", "./Resources/Textures/Menu/");
    connectScreen->setVisible(false);
    connectScreen->alpha = 1.0;
    connectScreen->setBackgroundTexture(bg, 0, 0);
    connectScreen->drawBackground = true;

	// Resize handles margins, 50 pixel spacing
	auto connectLayout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Middle, 0, 25);
	connectScreen->setLayout(connectLayout);

	// Game title
	auto gameTitle = new nanogui::Label(connectScreen, " ", "sans", 125);

	// Player name
	_playerNameBox = new nanogui::TextBox(connectScreen, "PlayerName");
	_playerNameBox->setEditable(true);
	_playerNameBox->setFontSize(38);
	_playerNameBox->setTextColor(nanogui::Color(233, 113, 136, 255));
	_playerNameBox->setAlignment(nanogui::TextBox::Alignment::Center);
    	//_playerNameBox->setBackgroundTexture(background, foreground1, foreground2);
    	//_playerNameBox->alpha = 0.5;
        //_playerNameBox->setTextColor(nanogui::Color(0, 0, 255, 255));

	// IP address box
	_addressBox = new nanogui::TextBox(connectScreen, "localhost");
	_addressBox->setEditable(true);
	_addressBox->setFontSize(38);
	_addressBox->setTextColor(nanogui::Color(233, 113, 136, 255));
	_addressBox->setAlignment(nanogui::TextBox::Alignment::Center);

	// Connect button
	_connectButton = new nanogui::Button(connectScreen, " ");
	_connectButton->setFontSize(28);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    auto unfocused = LoadTextureFromFile("connect.png", "./Resources/Textures/Menu/");
    auto focused = LoadTextureFromFile("connectfocused.png", "./Resources/Textures/Menu/");
    auto pushed = LoadTextureFromFile("connectpushed.png", "./Resources/Textures/Menu/");
    _connectButton->setBackgroundTexture(unfocused, focused, pushed);
    _connectButton->alpha = 1;
    _connectButton->setTheme(new nanogui::Theme(_screen->nvgContext()));
    _connectButton->theme()->mTextColor = nanogui::Color(0, 0, 255, 255);
	_connectButton->theme()->mTextColorShadow = nanogui::Color(0, 0, 0, 0);
	_connectButton->setFixedHeight(40);
	_connectButton->setFixedWidth(100);
		
}

void GuiManager::initLobbyScreen() {
	auto lobbyScreen = createWidget(_screen, WIDGET_LOBBY);

	// Lobby layout
	auto lobbyLayout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Middle, 50, 0);
	lobbyLayout->setRowAlignment(nanogui::Alignment::Middle);
	lobbyScreen->setLayout(lobbyLayout);

	/** Row 1 **/

	// Title for dogs list
	auto dogsLabel = new nanogui::Label(lobbyScreen, "Dogs", "sans", 60);

	// Controls button
	auto controlsButton = new nanogui::Button(lobbyScreen, "Controls");
	controlsButton->setCallback([&]()
		{
			setVisibility(WIDGET_OPTIONS, !getWidget(WIDGET_OPTIONS)->visible());
		});

	// Title for humans list
	auto humansLabel = new nanogui::Label(lobbyScreen, "Humans", "sans", 60);

	/** Row 2: Fixed-width padding **/

	int maxWidth = std::max(dogsLabel->preferredSize(_screen->nvgContext()).x(), humansLabel->preferredSize(_screen->nvgContext()).x());

	// Three empty widgets to fix size of each column
	auto emptyLabel1 = new nanogui::Label(lobbyScreen, "", "sans", 5);
	emptyLabel1->setFixedWidth(maxWidth);

	auto emptyLabel2 = new nanogui::Label(lobbyScreen, "", "sans", 5);

	auto emptyLabel3 = new nanogui::Label(lobbyScreen, "", "sans", 5);
	emptyLabel3->setFixedWidth(maxWidth);

	/** Row 3 **/

	// List of dogs
	auto dogsList = createWidget(lobbyScreen, WIDGET_LIST_DOGS);
	auto dogsLayout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 20);
	dogsList->setLayout(dogsLayout);

	// Switch sides button
	_switchSidesButton = new nanogui::Button(lobbyScreen, "Switch sides");
	auto switchUnfocused = LoadTextureFromFile("switch.png", "./Resources/Textures/Menu/");
	auto switchFocused = LoadTextureFromFile("switchfocused.png", "./Resources/Textures/Menu/");
	auto switchPushed = LoadTextureFromFile("switchdown.png", "./Resources/Textures/Menu/");
	_switchSidesButton->setBackgroundTexture(switchUnfocused, switchFocused, switchPushed);
	_switchSidesButton->alpha = 1;
	_switchSidesButton->setTheme(new nanogui::Theme(_screen->nvgContext()));
	_switchSidesButton->theme()->mTextColor = nanogui::Color(0, 0, 255, 255);
	_switchSidesButton->theme()->mTextColorShadow = nanogui::Color(0, 0, 0, 0);
	_switchSidesButton->setFixedHeight(40);
	_switchSidesButton->setFixedWidth(100);

	// List of humans
	auto humansList = GuiManager::getInstance().createWidget(lobbyScreen, WIDGET_LIST_HUMANS);
	auto humansLayout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 20);
	humansList->setLayout(humansLayout);

	/** Row 4: padding **/
	for (int i = 0; i < 3; i++) {
		new nanogui::Label(lobbyScreen, "", "sans", 5);
	}

	/** Row 5 **/

	// Empty label for padding in grid
	new nanogui::Label(lobbyScreen, "", "sans", 60);

	// Ready button
	_readyButton = new nanogui::Button(lobbyScreen, "");
	auto readyUnfocused = LoadTextureFromFile("ready.png", "./Resources/Textures/Menu/");
	auto readyFocused = LoadTextureFromFile("readyfocused.png", "./Resources/Textures/Menu/");
	auto readyPushed = LoadTextureFromFile("readydown.png", "./Resources/Textures/Menu/");
	_readyButton->setBackgroundTexture(readyUnfocused, readyFocused, readyPushed);
	_readyButton->alpha = 1;
	_readyButton->setTheme(new nanogui::Theme(_screen->nvgContext()));
	_readyButton->theme()->mTextColor = nanogui::Color(0, 0, 255, 255);
	_readyButton->theme()->mTextColorShadow = nanogui::Color(0, 0, 0, 0);
	_readyButton->setFixedHeight(40);
	_readyButton->setFixedWidth(100);
}

void GuiManager::initHUD() {
	// HUD container
	auto hudContainer = createWidget(_screen, WIDGET_HUD);
	auto hudLayout = new nanogui::BoxLayout(nanogui::Orientation::Vertical);
	hudContainer->setLayout(hudLayout);

	// Top HUD
	auto topHUD = createWidget(hudContainer, WIDGET_HUD_TOP);
	auto topHUDLayout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 3);
	topHUD->setLayout(topHUDLayout);

	// Empty left widget
	new nanogui::Label(topHUD, "", "sans", 5);

	// Clock
	_timer = new nanogui::Label(topHUD, "00:00.000", "sans", 52);
	_timer->setColor(SOLID_WHITE);

	// Empty right widget
	//new nanogui::Label(topHUD, "", "sans", 5);

	// Middle HUD
	auto middleHUD = createWidget(hudContainer, WIDGET_HUD_MIDDLE);
	auto middleHUDLayout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Middle, 0, 50);
	middleHUD->setLayout(middleHUDLayout);

	_primaryMessage = new nanogui::Label(middleHUD, "", "sans", 72);
	_primaryMessage->setColor(SOLID_WHITE);
	_secondaryMessage = new nanogui::Label(middleHUD, "", "sans", 48);
	_secondaryMessage->setColor(SOLID_WHITE);

	// Bottom HUD
	auto bottomHUD = createWidget(hudContainer, WIDGET_HUD_BOTTOM);
	auto bottomHUDLayout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Middle, 5);
	bottomHUD->setLayout(bottomHUDLayout);

	// Empty left widget
	new nanogui::Label(bottomHUD, "", "sans", 5);

	// Dog skills
	auto dogSkills= createWidget(bottomHUD, WIDGET_HUD_DOG_SKILLS);
	auto dogSkillsLayout = new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 50);
	dogSkills->setLayout(dogSkillsLayout);

	// Stamina
	_staminaInfo = new nanogui::Label(dogSkills, "Stamina: 100%", "sans", 32);
	_staminaInfo->setColor(SOLID_WHITE);

	// Pee
	_peeInfo = new nanogui::Label(dogSkills, "Pee: 100%", "sans", 32);
	_peeInfo->setColor(SOLID_WHITE);

	// Human skills
	auto humanSkills = createWidget(bottomHUD, WIDGET_HUD_HUMAN_SKILLS);
	auto humanSkillsLayout = new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 50);
	humanSkills->setLayout(humanSkillsLayout);

	// Plunger
	_plungerInfo = new nanogui::Label(humanSkills, "Plunger: Ready", "sans", 32);
	_plungerInfo->setColor(SOLID_HIGHLIGHTED);

	// Trap
	_trapInfo = new nanogui::Label(humanSkills, "Trap: Ready", "sans", 32);
	_trapInfo->setColor(SOLID_WHITE);

	// Charge rate of swinging
	_chargeInfo = new nanogui::Label(humanSkills, "Charge: 0%", "sans", 32);
	_chargeInfo->setColor(SOLID_WHITE);

	//auto skillsPlaceholder = new nanogui::Label(bottomHUD, "Skills go here", "sans", 52);
	//skillsPlaceholder->setColor(Color(Vector4f(1,1,1,0.2f)));

	// Empty right widget
	new nanogui::Label(bottomHUD, "", "sans", 5);
}

void GuiManager::initControlMenu() {
	auto controlsWidget = createWidget(_screen, WIDGET_OPTIONS);
	controlsWidget->setLayout(new nanogui::GroupLayout());

	auto controllerLabel = new Label(controlsWidget, "Select Controller", "sans", 16);
	_gamepadSelect = new nanogui::detail::FormWidget<GamePadIndex, std::integral_constant<bool, true>>(controlsWidget);
	_gamepadSelect->setItems({ "None", "1", "2", "3", "4" });

	auto windowCast = static_cast<nanogui::Window*>(controlsWidget);
}

void GuiManager::setVisibility(nanogui::Widget* widget, bool visibility) {
	widget->setVisible(visibility);
	for (auto& child : widget->children()) {
		setVisibility(child, visibility);
	}
}
