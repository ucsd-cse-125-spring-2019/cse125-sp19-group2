/**
 * Application.hpp
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
};

#endif