/**
 * Application.hpp
 */

#include "Application.hpp"
#include <iostream>
#include <fstream>
#include "Camera.hpp"
#include "InputManager.h"
#include "Shared/Logger.hpp"
#include "Shared/GameState.hpp"
#include "EntityManager.hpp"
#include "GuiManager.hpp"
#include "AudioManager.hpp"
#include "ColliderManager.hpp"
#include "ParticleSystemManager.hpp"
#include "CFloorEntity.hpp"

Application::Application(const char* windowTitle, int argc, char** argv) {
  _win_title = windowTitle;
  _win_width = 1280;
  _win_height = 720;

  if (argc == 1) {
  }
  else {
    Logger::getInstance()->fatal("Invalid number of arguments");
    fgetc(stdin);
    return;
  }

  // Create network client. Connection happens in UI button callback
  _networkClient = std::make_unique<NetworkClient>();

  // Initialize GLFW
  if (!glfwInit()) {
    Logger::getInstance()->fatal("Failed to initialize GLFW");
    return;
  }

  glfwSetErrorCallback(StaticError);
}

Application::~Application() {
  if (_window != nullptr) DestroyWindow();
  glfwTerminate();
}

void Application::Setup() {
  // OpenGL Graphics Setup
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
    

  // Initialize pointers
  _testShader = std::make_unique<Shader>();
  _quadShader = std::make_unique<Shader>();
  _skyboxShader = std::make_unique<Shader>();
  _debuglightShader = std::make_unique<Shader>();

  _camera = std::make_unique<Camera>();
  _camera->set_position(0, 0, 3.0f);
  _frameBuffer = std::make_unique<FrameBuffer>(800, 600, true);
  _quadFrameBuffer = std::make_unique<FrameBuffer>(800, 600);

  // Set up testing shader program
  _testShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/pano.vert");
  _testShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/pano.frag");
  _testShader->CreateProgram();

  // quad pass through shader
  _quadShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/quad.vert");
  _quadShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/quad.frag");
  _quadShader->CreateProgram();

  // Skybox shader
  _skyboxShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/skybox.vert");
  _skyboxShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/skybox.frag");
  _skyboxShader->CreateProgram();
  
  // Debugging shader for rendering lights
  _debuglightShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/debuglight.vert");
  _debuglightShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/debuglight.frag");
  _debuglightShader->CreateProgram();

  _skybox = std::make_unique<Skybox>("skybox");

  // Create light
  _dir_light = std::make_unique<DirectionalLight>(
    DirectionalLight{
      "u_dirlight",
      { { 0.05f, 0.01f, 0.01f }, { 0.8f, 0.3f, 0.3f }, { 1.0f, 0.5f, 0.5f } },
      { -1.0f, -1.0f, 0.0f }
    }
  );

    // Give InputManager a reference to GLFWwindow
    InputManager::getInstance().setWindow(_window);

    // Initialize GuiManager
    GuiManager::getInstance().init(_window);

	// Create widget screens
	GuiManager::getInstance().initWidgets();

	// Connect screen button callback
	GuiManager::getInstance().registerConnectCallback([&]() {
		// Attempt server connection
		std::string address = GuiManager::getInstance().getAddress();
		std::string playerName = GuiManager::getInstance().getPlayerName();

		// Save address and playername to session file
		std::ofstream ofs;
		ofs.open(SESSION_FILE_PATH, std::ofstream::out | std::ofstream::trunc);
		ofs << address << std::endl;
		ofs << playerName << std::endl;
		ofs.close();

		uint32_t playerId;
		try {
			playerId = _networkClient->connect(address, PORTNUM);
		}
		catch (std::runtime_error e) {
			Logger::getInstance()->error(e.what());
			return;
		}

		// Send join event
		auto joinEvent = std::make_shared<GameEvent>();
		joinEvent->playerId = playerId;
		joinEvent->type = EVENT_PLAYER_JOIN;
		joinEvent->playerName = playerName;
		_networkClient->sendEvent(joinEvent);

		// Create local player
		_localPlayer = std::make_unique<LocalPlayer>(playerId, _networkClient);

		// Register global keys
		registerGlobalKeys();

		// Hide connect screen
		GuiManager::getInstance().hideAll();

		// Show lobby
		GuiManager::getInstance().setVisibility(WIDGET_LOBBY, true);
	});

	// Connect screen default address and player name from session file
	std::ifstream ifs;
	ifs.open(SESSION_FILE_PATH);
	if (!ifs.fail()) {
		std::string address, playerName;
		ifs >> address;
		ifs >> playerName;
		ifs.close();
		GuiManager::getInstance().setAddress(address);
		GuiManager::getInstance().setPlayerName(playerName);
	}

	// Switch sides button callback
	GuiManager::getInstance().registerSwitchSidesCallback([&]() {
		// Send switch event
		auto switchEvent = std::make_shared<GameEvent>();
		switchEvent->type = EVENT_PLAYER_SWITCH;
		switchEvent->playerId = _localPlayer->getPlayerId();
		_networkClient->sendEvent(switchEvent);
	});

	GuiManager::getInstance().registerReadyCallback([&]() {
		// Send ready event
		auto readyEvent = std::make_shared<GameEvent>();
		readyEvent->type = EVENT_PLAYER_READY;
		readyEvent->playerId = _localPlayer->getPlayerId();
		_networkClient->sendEvent(readyEvent);

		// Gray out ready and switch buttons
		GuiManager::getInstance().setReadyEnabled(false);
		GuiManager::getInstance().setSwitchEnabled(false);
	});

	// Controls menu stuff
	GuiManager::getInstance().registerControllerCallback([=](GamePadIndex e) {
		switch (e) {
		case GamePadIndex_NULL:
			_localPlayer->setControllerNum(GamePadIndex_NULL);
			_gamepad_num = e;
			break;
		case GamePadIndex_One:
			if (_localPlayer->setControllerNum(GamePadIndex_One))
				_gamepad_num = e;
			break;
		case GamePadIndex_Two:
			if (_localPlayer->setControllerNum(GamePadIndex_Two))
				_gamepad_num = e;
			break;
		case GamePadIndex_Three:
			if (_localPlayer->setControllerNum(GamePadIndex_Three))
				_gamepad_num = e;
			break;
		case GamePadIndex_Four:
			if (_localPlayer->setControllerNum(GamePadIndex_Four))
				_gamepad_num = e;
			break;
		}
	});

	// Disconnect button
	GuiManager::getInstance().registerDisconnectCallback([&]() {
		_networkClient->closeConnection();
	});

	compassGUI = std::make_unique<CompassGUI>(_win_width, _win_height);

	_bgm = AudioManager::getInstance().getAudioSource("bgm");
	_bgm->init("Resources/Sounds/bgm1.mp3");
	_bgm->setVolume(0.05f);
	_bgm->play(true);
}

void Application::Cleanup() {
}

void Application::Run() {
  PreCreate();

  // Create the GLFW window
  _window = glfwCreateWindow(_win_width, _win_height, _win_title, NULL, NULL);

  // Check if the window could not be created
  if (!_window) {
    Logger::getInstance()->fatal("Failed to open GLFW window");
    Logger::getInstance()->fatal("Either GLFW is not installed or your " +
      std::string("graphics card does not support OpenGL"));
    glfwTerminate();
    fgetc(stdin);
    return;
  }

  PostCreate();

  // Load GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    Logger::getInstance()->fatal("Failed to initialize GLAD");
    fgetc(stdin);
    return;
  }

  Setup();

  glfwSwapInterval(1);
  glfwGetFramebufferSize(_window, &_win_width, &_win_height);
  StaticResize(_window, _win_width, _win_height);

  while (!glfwWindowShouldClose(_window)) {
    float current_frame = static_cast<float>(glfwGetTime());
    _delta_time = current_frame - _last_frame;
    _last_frame = current_frame;

    glfwPollEvents();
    Update();
    Draw();
    glfwSwapBuffers(_window);
  }

  Cleanup();
}

void Application::Update()
{
  // Get updates from controller
    if(_localPlayer) {
        _localPlayer->updateController();
    }

  // Get updates from the server
  try
  {
    for (auto& state : _networkClient->receiveUpdates())
    {
        // Update entity
        EntityManager::getInstance().update(state);

		// Update entity particle system
		ParticleSystemManager::getInstance().updateState(state);

        // Check for special case of GameState update
        if (state->type == ENTITY_STATE)
        {
            auto gameState = std::static_pointer_cast<GameState>(state);

			_serverEntityCount = gameState->entityCount;

			// Update client-side state and UI
            // Timer, winner of game, in lobby, etc
			if (gameState->inLobby)
			{
				GuiManager::getInstance().updateDogsList(
					gameState->dogs,
					_localPlayer->getPlayerId());

				GuiManager::getInstance().updateHumansList(
					gameState->humans,
					_localPlayer->getPlayerId());

				// Ready button text
				int numPlayers = gameState->dogs.size() + gameState->humans.size();
				GuiManager::getInstance().setReadyText("(" + std::to_string(gameState->readyPlayers.size()) + std::string("/") + std::to_string(numPlayers) + std::string(")"));

				// If everyone is ready, show loading screen
				if (numPlayers == gameState->readyPlayers.size())
				{
					GuiManager::getInstance().getWidget(WIDGET_LOBBY)->setVisible(false);
					GuiManager::getInstance().getWidget(WIDGET_OPTIONS)->setVisible(false);
					GuiManager::getInstance().getWidget(WIDGET_LOADING)->setVisible(true);

					// Get mute status first, then mute
					_muteSetting = AudioManager::getInstance().getMute();
					AudioManager::getInstance().setMute(true);

					// Start timer
					_startTime = std::chrono::steady_clock::now();
				}
			}

			// Did a game just start?
			if (_inLobby && !gameState->inLobby)
			{
				_inLobby = false;
				_gameLoaded = false;
			}
			// If everything is loaded, hide loading screen and show HUD
			else if (((!_inLobby && !gameState->waitingForClients && gameState->pregameCountdown) ||
				gameState->debugMode) &&
				GuiManager::getInstance().getWidget(WIDGET_LOADING)->visible())
			{
				GuiManager::getInstance().getWidget(WIDGET_LOADING)->setVisible(false);
				GuiManager::getInstance().setReadyEnabled(true);
				GuiManager::getInstance().setSwitchEnabled(true);

				// Back to user setting for audio mute
				AudioManager::getInstance().setMute(_muteSetting);

				// Capture mouse
				_localPlayer->setMouseCaptured(true);

				// Show game HUD
				GuiManager::getInstance().setVisibility(WIDGET_HUD, true);

				// Hide human skills if we are a dog
				for (auto& player : gameState->dogs)
				{
					if (player.first == _localPlayer->getPlayerId())
					{
						GuiManager::getInstance().setVisibility(WIDGET_HUD_HUMAN_SKILLS, false);
						break;
					}
				}

				// Hide dog skills if we are a human
				for (auto& player : gameState->humans)
				{
					if (player.first == _localPlayer->getPlayerId())
					{
						GuiManager::getInstance().setVisibility(WIDGET_HUD_DOG_SKILLS, false);
						break;
					}
				}

				// Redraw
				GuiManager::getInstance().refresh();
			}
			// Conversely, did a game just end and send us back to the lobby?
			else if (gameState->inLobby && !_inLobby)
			{
				// Deallocate world objects
				EntityManager::getInstance().clearAll();
				ColliderManager::getInstance().clear();
				ParticleSystemManager::getInstance().clear();

				// Stop playing sounds
				AudioManager::getInstance().reset();

				// Set localPlayer's _playerEntity to null
				_localPlayer->unpairEntity();

				// Reset UI
				_inLobby = true;
				GuiManager::getInstance().hideAll();
				GuiManager::getInstance().getWidget(WIDGET_LOBBY)->setVisible(true);
				GuiManager::getInstance().getWidget(WIDGET_LIST_DOGS)->setVisible(true);
				GuiManager::getInstance().getWidget(WIDGET_LIST_HUMANS)->setVisible(true);

				GuiManager::getInstance().setPrimaryMessage("");
				GuiManager::getInstance().setSecondaryMessage("");

				// Uncapture mouse
				_localPlayer->setMouseCaptured(false);

				// Redraw
				GuiManager::getInstance().refresh();
			}

			// Update pregame timer
			if (gameState->pregameCountdown)
			{
				_inCountdown = true;
				auto secondsLeft = (int)(std::ceil(gameState->millisecondsToStart / 1000.0f));

				if (secondsLeft <= 3)
				{
					GuiManager::getInstance().setPrimaryMessage("Get set!");
				}
				else
				{
					GuiManager::getInstance().setPrimaryMessage("On your marks!");

				}
				GuiManager::getInstance().setSecondaryMessage(std::to_string(secondsLeft));

				GuiManager::getInstance().refresh();
			}
			else if (_inCountdown)
			{
				_countdownEnd = std::chrono::steady_clock::now();
				_inCountdown = false;
				_startHidden = false;
				GuiManager::getInstance().setPrimaryMessage("Go!");
				GuiManager::getInstance().setSecondaryMessage("");

				GuiManager::getInstance().refresh();
			}
			else if (!_startHidden)
			{
				// Hide Go! message after 1 second
				auto elapsed = std::chrono::steady_clock::now() - _countdownEnd;

				if (std::chrono::duration_cast<std::chrono::seconds>(elapsed) >
					std::chrono::seconds(1))
				{
					GuiManager::getInstance().setPrimaryMessage("");
					_startHidden = true;

					GuiManager::getInstance().refresh();
				}
			}

			// Show winners if the game is over
			if (gameState->gameOver)
			{
				if (gameState->winner == ENTITY_DOG)
				{
					GuiManager::getInstance().setPrimaryMessage("Dogs Win!");
				}
				else if (gameState->winner == ENTITY_HUMAN)
				{
					GuiManager::getInstance().setPrimaryMessage("Humans Win!");
				}
				auto secondsLeft = (int)(std::ceil(gameState->millisecondsToLobby / 1000.0f));
				GuiManager::getInstance().setSecondaryMessage("Returning to lobby in " + std::to_string(secondsLeft) + "...");

				GuiManager::getInstance().refresh();
			}

			// Update countdown timer
			GuiManager::getInstance().updateTimer(gameState->millisecondsLeft);

			_inLobby = gameState->inLobby;
        }
    }
  }
  catch (std::runtime_error e)
  {
	  // Disconnected from the server; de-allocate client-side objects and
	  // show connection screen
	  _localPlayer = nullptr;
	  _networkClient->closeConnection();
	  EntityManager::getInstance().clearAll();
	  InputManager::getInstance().reset();
	  AudioManager::getInstance().reset();
	  ColliderManager::getInstance().clear();
	  ParticleSystemManager::getInstance().clear();
	  GuiManager::getInstance().hideAll();
	  GuiManager::getInstance().setVisibility(WIDGET_CONNECT, true);

	  // Un-capture mouse
 	  glfwSetInputMode(
		InputManager::getInstance().getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  InputManager::getInstance().update();

  if (_localPlayer) {
      _localPlayer->update();
  }

  // Update particle system physics
  ParticleSystemManager::getInstance().updatePhysics(0.016f);
    
  // Update sound engine
  AudioManager::getInstance().update();
    
  _camera->Update();
}

void Application::Draw() {
  // Begin drawing scene
  glViewport(0, 0, _win_width, _win_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // Test FBO
  _frameBuffer->renderScene([this]
  {
    // render scene
    //_frameBuffer->drawQuad(_testShader);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);

	  // Non-UI elements depend on localPlayer and that we're not in the lobby
	  if (_localPlayer && !_inLobby) {
		  // Render Skybox
		  _skyboxShader->Use();
		  _skyboxShader->set_uniform("u_projection", _localPlayer->getCamera()->projection_matrix());
		  _skyboxShader->set_uniform("u_view", _localPlayer->getCamera()->view_matrix() * glm::scale(glm::mat4(1.0f), glm::vec3(200, 200, 200)));
		  //glm::mat4(glm::mat3(_localPlayer->getCamera()->view_matrix()))
		  _skybox->draw(_skyboxShader);

		  // Render floor before any entity
		  CFloorEntity::getInstance().render(_localPlayer->getCamera());

          // Render particles
		  ParticleSystemManager::getInstance().render(_localPlayer->getCamera());

		  EntityManager::getInstance().render(_localPlayer->getCamera());

		  // Debug Shader
		  _debuglightShader->Use();
		  _debuglightShader->set_uniform("u_projection", _localPlayer->getCamera()->projection_matrix());
		  _debuglightShader->set_uniform("u_view", _localPlayer->getCamera()->view_matrix());

		  compassGUI->render();
	  }

    // Draw UI
    GuiManager::getInstance().draw();
  });

  // Render _frameBuffer Quad
  //_frameBuffer->drawQuad(_quadShader);

	_frameBuffer->blit(_quadFrameBuffer);
	_quadFrameBuffer->drawQuad(_quadShader);

  // Finish drawing scene
  glFinish();

  // If we rendered all the server entities, send a gameReady event
  if (!_gameLoaded && !_inLobby &&
	  EntityManager::getInstance().getEntityCount() >= _serverEntityCount) {
	  auto readyEvent = std::make_shared<GameEvent>();
	  readyEvent->type = EVENT_CLIENT_READY;
	  readyEvent->playerId = _localPlayer->getPlayerId();
	  try {
		  _networkClient->sendEvent(readyEvent);
	  }
	  catch (std::runtime_error e) {};

	  _gameLoaded = true;

	  // Trigger a manual resize
	  auto screen = GuiManager::getInstance().getScreen();
	  StaticResize(_window, screen->size().x(), screen->size().y());
  }
  // Edge case in which not all state was received by the client
  else if (!_gameLoaded && !_inLobby &&
	  EntityManager::getInstance().getEntityCount() > 0 &&
	  EntityManager::getInstance().getEntityCount() < _serverEntityCount) {

	  // Check time since start
	  auto elapsed = std::chrono::steady_clock::now() - _startTime;
	  if (std::chrono::duration_cast<std::chrono::seconds>(elapsed) > std::chrono::seconds(4))
	  {
		  // Build a list of the entities we have
		  auto entityList = EntityManager::getInstance().getEntityIdList();

		  // Request a re-send from the server
		  auto resendEvent = std::make_shared<GameEvent>();
		  resendEvent->type = EVENT_REQUEST_RESEND;
		  resendEvent->playerId = _localPlayer->getPlayerId();
		  resendEvent->entityList = entityList;
		  try {
			  _networkClient->sendEvent(resendEvent);
		  }
		  catch (std::runtime_error e) {};

		  // Reset timer
		  _startTime = std::chrono::steady_clock::now();
	  }
  }
}

void Application::Reset() {
}

void Application::StaticError(int error, const char* description) {
  Logger::getInstance()->error(std::string(description));
}

void Application::StaticResize(GLFWwindow* window, int x, int y) {
  Application* instance = (Application *)glfwGetWindowUserPointer(window);
  instance->Resize(x, y);
}

void Application::StaticKeyboard(GLFWwindow* window,
  int key, int scancode, int action, int mods) {
  Application* instance = (Application *)glfwGetWindowUserPointer(window);
  instance->Keyboard(key, scancode, action, mods);

}

void Application::StaticMouseButton(GLFWwindow* window, int btn, int action, int mods) {
  Application* instance = (Application *)glfwGetWindowUserPointer(window);
  instance->MouseButton(btn, action, mods);
}

void Application::StaticMouseMotion(GLFWwindow* window, double x, double y) {
  Application* instance = (Application *)glfwGetWindowUserPointer(window);
  instance->MouseMotion(x, y);
}

void Application::StaticMouseScroll(GLFWwindow* window, double x, double y) {
  Application* instance = (Application *)glfwGetWindowUserPointer(window);
  instance->MouseScroll(x, y);
}

void Application::Resize(int x, int y) {
  glfwGetFramebufferSize(_window, &x, &y);
  _win_width = x;
  _win_height = y;
  glViewport(0, 0, x, y);
  _frameBuffer->resize(x, y);
	_quadFrameBuffer->resize(x, y);
	compassGUI->updateWindowSize(x, y);
  if (_localPlayer) {
	  _localPlayer->resize(x, y);
  }
  GuiManager::getInstance().resize(x, y);
}

void Application::Keyboard(int key, int scancode, int action, int mods) {
  GuiManager::getInstance().getScreen()->keyCallbackEvent(key, scancode, action, mods);
  if (action == GLFW_PRESS) {
    if (mods == GLFW_MOD_SHIFT) {
      InputManager::getInstance().fire(key, KeyState::Press | KeyState::Shift);
    }
    else {
      InputManager::getInstance().fire(key, KeyState::Press);
    }
  }
  else if (action == GLFW_RELEASE) {
	  InputManager::getInstance().fire(key, KeyState::Release);
  }
}

void Application::MouseButton(int btn, int action, int mods) {
	GuiManager::getInstance().getScreen()->mouseButtonCallbackEvent(btn, action, mods);

	if(action == GLFW_PRESS)
	{
		if(mods == GLFW_MOD_SHIFT)
		{
			InputManager::getInstance().fire(btn, KeyState::Press | KeyState::Shift);
		}else
		{
			InputManager::getInstance().fire(btn, KeyState::Press);
		}
	}else if(action == GLFW_RELEASE)
	{
		InputManager::getInstance().fire(btn, KeyState::Release);
	}
}

void Application::MouseMotion(double x, double y) {
    GuiManager::getInstance().getScreen()->cursorPosCallbackEvent(x, y);
    InputManager::getInstance().move(Key::KEYTYPE::MOUSE, x, y);
}

void Application::MouseScroll(double x, double y) {
    GuiManager::getInstance().getScreen()->scrollCallbackEvent(x, y);
    InputManager::getInstance().scroll(y);
}

void Application::PreCreate() {
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  glfwWindowHint(GLFW_SAMPLES, 0);
  glfwWindowHint(GLFW_RED_BITS, 8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS, 8);
  glfwWindowHint(GLFW_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
}

void Application::PostCreate() {
  glfwSetWindowUserPointer(_window, this); // Keep track of Application instance
  glfwSetKeyCallback(_window, StaticKeyboard);
  glfwSetMouseButtonCallback(_window, StaticMouseButton);
  glfwSetCursorPosCallback(_window, StaticMouseMotion);
  glfwSetScrollCallback(_window, StaticMouseScroll);
  glfwSetFramebufferSizeCallback(_window, StaticResize);
  glfwMakeContextCurrent(_window);
}

void Application::DestroyWindow() {
  glfwSetKeyCallback(_window, nullptr);
  glfwSetMouseButtonCallback(_window, nullptr);
  glfwSetCursorPosCallback(_window, nullptr);
  glfwSetFramebufferSizeCallback(_window, nullptr);
  glfwDestroyWindow(_window);
}

void Application::registerGlobalKeys() {
	// Test input; to be removed
	InputManager::getInstance().getKey(GLFW_KEY_G)->onPress([&]
	{
		std::cout << "Hello World!" << this->count << std::endl;
	});

	InputManager::getInstance().getKey(GLFW_KEY_K)->onRepeat([&]
	{
		this->count += 1;
		std::cout << this->count << std::endl;
	});

	InputManager::getInstance().getKey(GLFW_KEY_T)->onPress([&]
	{
		ColliderManager::getInstance().renderMode = !ColliderManager::getInstance().renderMode;
	});

	InputManager::getInstance().getKey(GLFW_KEY_ESCAPE)->onPress([&]
	{
		if (_localPlayer) {
			bool curState = _localPlayer->getMouseCaptured();
			GuiManager::getInstance().setVisibility(WIDGET_OPTIONS,
				!GuiManager::getInstance().getWidget(WIDGET_OPTIONS)->visible());
			if (!_inLobby)
			{
				_localPlayer->setMouseCaptured(!curState);
			}
		}
	});
}
