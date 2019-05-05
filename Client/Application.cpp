/**
 * Application.hpp
 */

#include "Application.hpp"
#include <iostream>
#include "Camera.hpp"
#include "InputManager.h"
#include "Shared/Logger.hpp"
#include "EntityManager.hpp"
#include "GuiManager.h"

void vec3ToFmod(const glm::vec3& v, FMOD_VECTOR* fv)
{
    fv->x = v.x;
    fv->y = v.y;
    fv->z = v.z;
}

Application::Application(const char* windowTitle, int argc, char** argv) {
  _win_title = windowTitle;
  _win_width = 800;
  _win_height = 600;

  if (argc == 1) {
  }
  else {
    Logger::getInstance()->fatal("Invalid number of arguments");
    fgetc(stdin);
    return;
  }

  // Create network client and connect to server. The connect logic should
  // eventually be moved into the main game loop, but we're not there yet
  _networkClient = std::make_unique<NetworkClient>();
  try
  {
    uint32_t playerId = _networkClient->connect("localhost", PORTNUM);

	// First thing we do is create a player join event with player name
	auto joinEvent = std::make_shared<GameEvent>();
	joinEvent->playerId = playerId;
	joinEvent->type = EVENT_PLAYER_JOIN;

	// TODO: change this to player-specified name
	joinEvent->playerName = "Player" + std::to_string(playerId);
	_networkClient->sendEvent(joinEvent);

	_localPlayer = std::make_unique<LocalPlayer>(playerId, _networkClient);
  }
  catch (std::runtime_error e)
  {
    Logger::getInstance()->error(e.what());
  }

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
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glEnable(GL_CULL_FACE);

    

  // Initialize pointers
  _testShader = std::make_unique<Shader>();
  _quadShader = std::make_unique<Shader>();
  _skyboxShader = std::make_unique<Shader>();
  _debuglightShader = std::make_unique<Shader>();

  _camera = std::make_unique<Camera>();
  _camera->set_position(0, 0, 3.0f);
  _frameBuffer = std::make_unique<FrameBuffer>(800, 600);
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

  _skybox = std::make_unique<Skybox>("thefog");

  // Create light
  _point_light = std::make_unique<PointLight>(
    PointLight{
      "u_pointlight",
      { { 0.05f, 0.05f, 0.05f }, { 0.8f, 0.8f, 0.8f }, { 1.0f, 1.0f, 1.0f } },
      { -1.0f, 0.0f, 0.0f },
      { 1.0f, 0.09f, 0.032f }
    }
  );
  _dir_light = std::make_unique<DirectionalLight>(
    DirectionalLight{
      "u_dirlight",
      { { 0.05f, 0.01f, 0.01f }, { 0.8f, 0.3f, 0.3f }, { 1.0f, 0.5f, 0.5f } },
      { -1.0f, -1.0f, 0.0f }
    }
  );

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

    // Initialize GuiManager
    GuiManager::getInstance().init(_window);

    // Create a testing widget
    bool enabled = true;
    auto *gui = GuiManager::getInstance().createFormHelper("Form helper");
    gui->addWindow(Eigen::Vector2i(10, 10), "Form helper")->center();
    
    gui->addGroup("Validating fields");
    auto * v = gui->addVariable("int", _integer);
    v->setSpinnable(true);

    gui->addVariable("float", _float)->setTooltip("Test.");
    gui->addButton("Add One", [&]() {
        _integer += 1;
    });

    
    // Test Sound
    FMOD_RESULT result;

    result = FMOD::System_Create(&_system); // Create the main system object.
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    }

    result = _system->init(512, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0); // Initialize FMOD.
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    }

    _system->setSpeakerPosition(FMOD_SPEAKER_FRONT_LEFT, -1.0f, 0.0f, true);
    _system->setSpeakerPosition(FMOD_SPEAKER_FRONT_RIGHT, 1.0f, 0.0f, true);
    _system->setDSPBufferSize(512, 2);

    // listener init
    _pos = new FMOD_VECTOR();
    _forward = new FMOD_VECTOR();
    _up = new FMOD_VECTOR();

    // Ambient sound setup
    result = _system->createSound(
        "Resources/Sounds/Mario - Overworld.mp3", // Dummy background music
        FMOD_LOOP_NORMAL ,
        NULL,
        &_ambientAudio
    );


    result = _system->playSound(
        _ambientAudio,
        NULL,
        true,
        &_ambientChannel
    );

    _ambientChannel->setVolume(0.4);
    _ambientChannel->setPaused(!_playAmbient);


    // Positional sound setup
    result = _system->createSound(
        "Resources/Sounds/footsteps.mp3", // Dummy background music
        FMOD_LOOP_NORMAL | FMOD_3D | FMOD_3D_WORLDRELATIVE | FMOD_3D_INVERSEROLLOFF,
        NULL,
        &_positionalAudio
    );


    result = _system->playSound(
        _positionalAudio,
        NULL,
        true,
        &_positionalChannel
    );

    _positionalChannel->setVolume(1.0);

    // Set Listener's position
    vec3ToFmod(glm::vec3(0,0,0), _pos);
    vec3ToFmod(glm::vec3(0,0,-1), _forward);
    vec3ToFmod(glm::vec3(0,1,0), _up);
    _system->set3DListenerAttributes(0, _pos, 0, _forward, _up);
    
    gui->addGroup("Sound");
    gui->addVariable("Play Ambient Sound", _playAmbient)->setCallback([&](const bool flag) {
        _playAmbient = flag;
        _ambientChannel->setPaused(!flag);
    });
    gui->addVariable("Play Positional Sound", _playPositional)->setCallback([&](const bool flag) {
        _playPositional = flag;
        _positionalChannel->setPaused(!flag);
    });
    auto * pos = gui->addVariable("Sound Location", _enum, enabled);
    pos->setItems({ "Left", "Front", "Right" });
    pos->setCallback([&](const Enum & e) {
        FMOD_VECTOR v;
        switch (e) {
		case Left:
            vec3ToFmod(glm::vec3(-2, 0 , 0), &v);
            break;
		case Front:
            vec3ToFmod(glm::vec3(0, 0 , -2), &v);
            break;
		case Right:
            vec3ToFmod(glm::vec3(2, 0 , 0), &v);
            break;
        }
        _enum = e;
        _positionalChannel->set3DAttributes(&v, 0);
    });
    
    GuiManager::getInstance().setDirty();
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

  glfwSwapInterval(0);
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
    // Update sound position
    _system->update();

  // Get updates from the server
  try
  {
    for (auto& state : _networkClient->receiveUpdates())
    {
        // Update entity
		EntityManager::getInstance().update(state);
    }
  }
  catch (std::runtime_error e)
  {
    // Disconnected from the server
  }
  if (_localPlayer) {
	  _localPlayer->update();
  }
  InputManager::getInstance().update();
  _camera->Update();
  _point_light->update();
}

void Application::Draw() {
  // Begin drawing scene
  glViewport(0, 0, _win_width, _win_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // Test FBO
  _quadFrameBuffer->renderScene([this]
  {
    // render scene
    //_frameBuffer->drawQuad(_testShader);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);
	// Render Skybox
	_skyboxShader->Use();
	_skyboxShader->set_uniform("u_projection", _localPlayer->getCamera()->projection_matrix());
	_skyboxShader->set_uniform("u_view", _localPlayer->getCamera()->view_matrix() * glm::scale(glm::mat4(1.0f), glm::vec3(200,200,200)));
      //glm::mat4(glm::mat3(_localPlayer->getCamera()->view_matrix()))
	_skybox->draw(_skyboxShader);

    EntityManager::getInstance().render(_localPlayer->getCamera());

	  // Debug Shader
    _debuglightShader->Use();
    _debuglightShader->set_uniform("u_projection", _localPlayer->getCamera()->projection_matrix());
    _debuglightShader->set_uniform("u_view", _localPlayer->getCamera()->view_matrix());
    _point_light->draw(_debuglightShader);

    // Draw UI
    GuiManager::getInstance().draw();
  });

  // Render _frameBuffer Quad
  _quadFrameBuffer->drawQuad(_quadShader);

  // Finish drawing scene
  glFinish();
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
    GuiManager::getInstance().getScreen()->resizeCallbackEvent(x, y);
  glfwGetFramebufferSize(_window, &x, &y);
  _win_width = x;
  _win_height = y;
  glViewport(0, 0, x, y);
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
    if (mods == GLFW_MOD_SHIFT) {

    }
    else {
      InputManager::getInstance().fire(key, KeyState::Release);
    }
  }
}

void Application::MouseButton(int btn, int action, int mods) {
    GuiManager::getInstance().getScreen()->mouseButtonCallbackEvent(btn, action, mods);
}

void Application::MouseMotion(double x, double y) {
    GuiManager::getInstance().getScreen()->cursorPosCallbackEvent(x, y);
        
}

void Application::MouseScroll(double x, double y) {
    GuiManager::getInstance().getScreen()->scrollCallbackEvent(x, y);
        
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
