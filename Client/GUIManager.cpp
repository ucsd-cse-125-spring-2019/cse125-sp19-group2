#include "GuiManager.hpp"
#include "Shared/Logger.hpp"
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
		//_screen->performLayout();

		// Redraw all children
		redraw(_screen);

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

void GuiManager::redraw(nanogui::Widget* widget) {
	for (auto& child : widget->children()) {
		redraw(child);
	}
	if (widget->layout()) {
		widget->layout()->performLayout(_screen->nvgContext(), widget);
	}
}

void GuiManager::resize(int x, int y) {
	_screen->resizeCallbackEvent(x, y);

	// Resize our widgets
	for (auto& widgetPair : _widgets) {
		// 1/3 of the screen for dog/human lists, full screen otherwise
		if (widgetPair.first == WIDGET_LIST_DOGS || widgetPair.first == WIDGET_LIST_HUMANS) {
			widgetPair.second->setSize(nanogui::Vector2i(x / _screen->pixelRatio() / 3, y / _screen->pixelRatio() / 2));
		}
		else {
			widgetPair.second->setSize(nanogui::Vector2i(x / _screen->pixelRatio(), y / _screen->pixelRatio()));
		}

		// Resize layout margins
		switch (widgetPair.first) {
		case WIDGET_CONNECT:
			static_cast<nanogui::BoxLayout*>(widgetPair.second->layout())->setMargin(x * CONNECT_MARGIN);
			break;
		}

		_dirty = true;
	}
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

nanogui::Widget* GuiManager::createWidget(WidgetType name) {
	nanogui::Widget* widget;
	
	if (name == WIDGET_LIST_DOGS || name == WIDGET_LIST_HUMANS) {
		widget = new nanogui::Widget(getWidget(WIDGET_LOBBY));
		auto listLayout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 0, 20);
		widget->setLayout(listLayout);
	}
	else {
		widget = new nanogui::Widget(_screen);
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

std::string GuiManager::getPlayerName() {
	return _playerNameBox->value();
}

std::string GuiManager::getAddress() {
	return _addressBox->value();
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
				playerLabel->setColor(nanogui::Color(0.376f, 0.863f, 1.0f, 1.0f));
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
				playerLabel->setColor(nanogui::Color(0.376f, 0.863f, 1.0f, 1.0f));
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
	auto connectScreen = createWidget(WIDGET_CONNECT);

	// Resize handles margins, 50 pixel spacing
	auto connectLayout = new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Middle, 0, 25);
	connectScreen->setLayout(connectLayout);

	// Game title
	auto gameTitle = new nanogui::Label(connectScreen, "Project Bone", "sans", 72);

	// Player name
	_playerNameBox = new nanogui::TextBox(connectScreen, "PlayerName");
	_playerNameBox->setEditable(true);
	_playerNameBox->setFixedSize(nanogui::Vector2i(300, 50));
	_playerNameBox->setFontSize(38);
	_playerNameBox->setAlignment(nanogui::TextBox::Alignment::Center);

	// IP address box
	_addressBox = new nanogui::TextBox(connectScreen, "localhost");
	_addressBox->setEditable(true);
	_addressBox->setFixedSize(nanogui::Vector2i(300, 50));
	_addressBox->setFontSize(38);
	_addressBox->setAlignment(nanogui::TextBox::Alignment::Center);

	// Connect button
	_connectButton = new nanogui::Button(connectScreen, "Connect");
	_connectButton->setFontSize(28);
}

void GuiManager::initLobbyScreen() {
	auto lobbyScreen = createWidget(WIDGET_LOBBY);

	// Lobby layout
	auto lobbyLayout = new nanogui::GridLayout(nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Middle, 0, 50);
	lobbyLayout->setRowAlignment(nanogui::Alignment::Middle);
	lobbyScreen->setLayout(lobbyLayout);

	// Title for dogs list
	auto dogsLabel = new nanogui::Label(lobbyScreen, "Dogs", "sans", 60);

	// Empty widget
	auto emptyLabel = new nanogui::Label(lobbyScreen, "", "sans", 60);

	// Title for humans list
	auto humansLabel = new nanogui::Label(lobbyScreen, "Humans", "sans", 60);

	// List of dogs
	auto dogsList = createWidget(WIDGET_LIST_DOGS);

	// Switch sides button
	_switchSidesButton = new nanogui::Button(lobbyScreen, "Switch sides");

	// List of humans
	auto humansList = GuiManager::getInstance().createWidget(WIDGET_LIST_HUMANS);

	// Empty label for padding in grid
	new nanogui::Label(lobbyScreen, "", "sans", 60);

	// Ready button
	_readyButton = new nanogui::Button(lobbyScreen, "");
}

void GuiManager::initHUD() {
	// TODO
}

void GuiManager::initControlMenu() {
	// TODO
}

void GuiManager::setVisibility(nanogui::Widget* widget, bool visibility) {
	widget->setVisible(visibility);
	for (auto& child : widget->children()) {
		setVisibility(child, visibility);
	}
}