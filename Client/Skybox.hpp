#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Shader.hpp"

/**
 * \brief A class that loads a cubemap and render it as skybox
 */
class Skybox {
public:

  /**
	 * \brief Loads textures and setup Skybox
	 * \param dir(const std::string&) The folder of cubemap textures
	 */
  Skybox(const std::string& dir);
	~Skybox();

  /**
	 * \brief Render skybox
	 * \param shader(std::unique_ptr<Shader> const&) The shader program for rendering
	 */
  void draw(std::unique_ptr<Shader> const &shader) const;

private:
	// These variables are needed for the shader program
	unsigned int VBO, VAO;
	unsigned int texture;
};

