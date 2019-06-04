#include "CompassGUI.hpp"

CompassGUI::CompassGUI(float width, float height)
{
	// Allocate member variables
	_compassQuad = std::make_unique<Model>("./Resources/Models/compass.fbx");
	_objectShader = std::make_unique<Shader>();

	_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/compass.vert");
	_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/compass.frag");
	_objectShader->CreateProgram();

	updateWindowSize(width, height);
}

void CompassGUI::updateWindowSize(float width, float height) {
	this->width = width;
	this->height = height;
	ortho = glm::ortho(0.0f, width, height, 0.0f);
}

void CompassGUI::render() {
	glDisable(GL_DEPTH_TEST);

	glm::mat4 _guiMVP;

	_objectShader->set_uniform("u_projection", ortho);
	_objectShader->set_uniform("u_view", glm::mat4(1));

	// Compute model matrix based on state: t * r * s
	const auto t = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -1.0f));
	//const auto r = glm::lookAt(glm::vec3(0.0f), _state->forward, _state->up);
	const auto s = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 1.0f));

	auto model = t * s;

	// Pass model matrix into shader
	_objectShader->set_uniform("u_model", model);
	

	glEnable(GL_DEPTH_TEST); // Enable the Depth-testing
}