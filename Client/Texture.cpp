#include "Texture.hpp"

GLuint LoadTextureFromFile(const char *path, const std::string &directory)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	GLuint id;
	glGenTextures(1, &id);

	int width, height, num_components;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &num_components, 0);
	if (data)
	{
		GLenum format;
		if (num_components == 1)
			format = GL_RED;
		else if (num_components == 3)
			format = GL_RGB;
		else if (num_components == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "[ERROR] Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return id;
}