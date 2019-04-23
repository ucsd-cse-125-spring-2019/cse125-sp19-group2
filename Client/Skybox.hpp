#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include "Shader.hpp"

class Skybox {
public:

	Skybox(const std::string& dir);
	~Skybox();

	void draw(std::unique_ptr<Shader> const &shader) const;

	// These variables are needed for the shader program
	unsigned int VBO, VAO;
	unsigned int texture;
};

