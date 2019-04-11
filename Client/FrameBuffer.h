/**
 * FrameBuffer.h
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include "Shader.hpp"
#include <memory>

class FrameBuffer {
public:

  FrameBuffer();
  FrameBuffer(int width, int height);
  ~FrameBuffer();

  template <typename T>
  void renderScene(T&& lambda);

  void setEnable(bool status);
  void drawQuad(std::unique_ptr<Shader> const &quadShader, unsigned int externalTexture = -1);
  GLuint getRGB();
  GLuint getDepth();

private:

  GLuint frameBufferID, depthBufferID;
  GLuint rgbTexture, depthTexture;
  GLuint VAO, VBO;
  GLuint width, height;
};

template <typename T>
void FrameBuffer::renderScene(T&& lambda) {
  setEnable(true);
  lambda();
  setEnable(false);
}
#endif
