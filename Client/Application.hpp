/**
 * Application.hpp
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "Shader.hpp"
#include "Camera.hpp"
#include "FrameBuffer.h"
#include "NetworkClient.hpp"
#include "Model.hpp"
#include "Skybox.hpp"
#include "LocalPlayer.hpp"
#include "PointLight.hpp"
#include "DirectionalLight.hpp"
#include "AnimatedMesh.hpp"
#include "Animation.hpp"
#include "fmod/fmod.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "Shared/GameEvent.hpp"

class Application {
public:
  Application(const char *windowTitle, int argc, char **argv);
  ~Application();

  void Run();
  void Update();
  void Draw();

  void Reset();

  // Static event handlers
  static void StaticError(int error, const char* description);
  static void StaticResize(GLFWwindow* window, int x, int y);
  static void StaticKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void StaticMouseButton(GLFWwindow* window, int btn, int action, int mods);
  static void StaticMouseMotion(GLFWwindow* window, double x, double y);
  static void StaticMouseScroll(GLFWwindow* window, double x, double y);

  // Instance event handlers
  void Resize(int x, int y);
  void Keyboard(int key, int scancode, int action, int mods);
  void MouseButton(int btn, int action, int mods);
  void MouseMotion(double x, double y);
  void MouseScroll(double x, double y);

private:
  // Window management functions
  void PreCreate();
  void PostCreate();
  void DestroyWindow();

  // Environment management
  void Setup();
  void Cleanup();

  // Window management data
  GLFWwindow* _window = nullptr;
  const char* _win_title;
  int _win_width, _win_height;

  // Frame management
  float _delta_time = 0;
  float _last_frame = 0;

  // Network client
  std::unique_ptr<NetworkClient> _networkClient;

  // Player ID
  uint32_t _playerId;

  // Test Shader
  std::unique_ptr<Shader> _testShader;
  std::unique_ptr<Shader> _quadShader;
  std::unique_ptr<Shader> _cubeShader;
  std::unique_ptr<Shader> _skyboxShader;
  std::unique_ptr<Shader> _debuglightShader;

  // Test camera
  std::unique_ptr<Camera> _camera;

  // Test frameBuffer
  std::unique_ptr<FrameBuffer> _frameBuffer;
  std::unique_ptr<FrameBuffer> _quadFrameBuffer;

  // Test input capture
  int count = 0;

  // Test Model
  std::unique_ptr<Model> _cube;

  // Test Skybox
  std::unique_ptr<Skybox> _skybox;
  
  // Local player
  std::unique_ptr<LocalPlayer> _localPlayer;

  // UI
  nanogui::TextBox* _ipTextBox;
  nanogui::TextBox* _playerNameTextBox;

  // Test Lights
  std::unique_ptr<PointLight> _point_light;
  std::unique_ptr<DirectionalLight> _dir_light;

    // Test UI
    bool _playAmbient = false;
    bool _playPositional;
    int _integer;
    float _float;
    std::string _string;
    enum Enum {
        Left = 0,
        Front,
        Right
    };
    Enum _enum;
	GamePadIndex _gamepad_num;
	PlayerType _player_type;
};

#endif
