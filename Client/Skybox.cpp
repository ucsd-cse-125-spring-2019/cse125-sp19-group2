#include "Skybox.hpp"

#include <iostream>
#include <glad/glad.h>
#include "stb_image.h"

float skyboxVertices[] = {
  // positions          
  -1.0f, 1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,

  -1.0f, -1.0f, 1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f,

  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,

  -1.0f, -1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, -1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f,

  -1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, -1.0f,

  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, 1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, 1.0f,
  1.0f, -1.0f, 1.0f
};

std::vector<std::string> faces
{
  "front.tga",
  "back.tga",
  "up.tga",
  "down.tga",
  "right.tga",
  "left.tga"
};

unsigned loadCubemap(const std::string & dir) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, num_components;
  const std::string prefix = "./Resources/Textures/" + dir + "/";
  for (unsigned int i = 0; i < faces.size(); i++) {

    std::string filename = prefix + faces[i];
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &num_components, 3);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
      );
    }
    else {
      std::cout << "Cubemap texture failed to load at path: " << filename.c_str() << std::endl;
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

Skybox::Skybox(const std::string& dir) {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(0));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  texture = loadCubemap(dir);
}

Skybox::~Skybox() {
	glDeleteTextures(1, &texture);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &VAO);
}

void Skybox::draw(std::unique_ptr<Shader> const& shader) const {
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glDepthMask(GL_FALSE);
  shader->Use();
  glBindVertexArray(VAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  shader->set_uniform("skybox", 0);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthMask(GL_TRUE);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
