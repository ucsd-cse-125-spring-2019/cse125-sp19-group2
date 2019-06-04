#pragma once
#include "Model.hpp"
#include "Shader.hpp"

class CompassGUI
{
private:
	std::unique_ptr<Model> _compassQuad;
	// Shader program for object
	std::unique_ptr<Shader> _objectShader;
	float width, height;
	glm::mat4 ortho;

public:

	CompassGUI(float width, float height);

	void updateWindowSize(float width, float height);

	void render();
};

