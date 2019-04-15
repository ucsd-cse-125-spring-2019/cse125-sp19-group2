#include "FrameBuffer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <iostream>
#include "Shader.hpp"

static const GLfloat g_quad_vertex_buffer_data[] = {
  -1.0f, -1.0f, 0.0f,
  1.0f, -1.0f, 0.0f,
  -1.0f, 1.0f, 0.0f,
  -1.0f, 1.0f, 0.0f,
  1.0f, -1.0f, 0.0f,
  1.0f, 1.0f, 0.0f,
};

FrameBuffer::FrameBuffer() : FrameBuffer(1024, 1024) {

}

FrameBuffer::FrameBuffer(int width, int height) {
  this->width = width;
  this->height = height;
  glGenFramebuffers(1, &frameBufferID);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);

  glGenTextures(1, &rgbaTexture);

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, rgbaTexture);

  // Give an empty image to OpenGL ( the last "0" )
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  // Poor filtering. Needed !
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float color[] = {0.0f, 0.0f, 0.0f, 0.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

  glGenRenderbuffers(1, &depthBufferID);
  glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

  glGenTextures(1, &depthTexture);
  glBindTexture(GL_TEXTURE_2D, depthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rgbaTexture, 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

  // Set the list of draw buffers.
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3, // size
    GL_FLOAT, // type
    GL_FALSE, // normalized?
    3 * sizeof(GLfloat), // stride
    (void*)0 // array buffer offset
  );
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

FrameBuffer::~FrameBuffer() {
}

void FrameBuffer::setEnable(bool status) {
  if (status) {
    // Render to our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
  }
  else {
    // Render to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

void FrameBuffer::drawQuad(std::unique_ptr<Shader> const &quadShader, unsigned int externalTexture) {
  quadShader->Use();
  glBindVertexArray(VAO);
  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  if (externalTexture == -1) {
    glBindTexture(GL_TEXTURE_2D, rgbaTexture);
  }
  else {
    glBindTexture(GL_TEXTURE_2D, externalTexture);
  }

  // Set our "renderedTexture" sampler to use Texture Unit 0
  quadShader->set_uniform("rgbTexture", 0);

  // Draw the triangles 
  glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
  glBindVertexArray(0);
}

unsigned FrameBuffer::getRGBA() {
  return rgbaTexture;
}

unsigned FrameBuffer::getDepth() {
  return depthTexture;
}
