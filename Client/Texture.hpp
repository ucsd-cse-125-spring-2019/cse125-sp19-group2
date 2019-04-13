/**
 * Texture.hpp
 */

#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "stb_image.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <iostream>

enum class TextureType : unsigned char
{
	DIFFUSE,
	SPECULAR
};

struct Texture
{
	unsigned int id;
	TextureType type;
	std::string path;
};

GLuint LoadTextureFromFile(const char *path, const std::string &directory);

#endif