#include "FrameBuffer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <iostream>
#include "Shader.hpp"
#include "Shared/Logger.hpp"

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

FrameBuffer::FrameBuffer(int width, int height, bool multisample) {
  this->width = width;
  this->height = height;
	this->multisample = multisample;
  glGenFramebuffers(1, &frameBufferID);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);

  glGenTextures(1, &rgbaTexture);

	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };

  // "Bind" the newly created texture : all future texture functions will modify this texture
	if (multisample)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rgbaTexture);

		// Give an empty image to OpenGL ( the last "0" )
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, this->width, this->height, GL_TRUE);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BORDER_COLOR, color);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, rgbaTexture, 0);

		glGenRenderbuffers(1, &depthBufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, this->width, this->height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTexture);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT24, this->width, this->height, GL_TRUE);
		
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BORDER_COLOR, color);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTexture, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, rgbaTexture);

		// Give an empty image to OpenGL ( the last "0" )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rgbaTexture, 0);

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

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Logger::getInstance()->info("Framebuffer not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
    glDeleteFramebuffers(1, &frameBufferID);
    glDeleteTextures(1, &rgbaTexture);
    glDeleteTextures(1, &depthTexture);
    glDeleteRenderbuffers(1, &depthBufferID);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VAO);
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

void FrameBuffer::resize(int x, int y) {
    this->width = x;
    this->height = y;

		if (this->multisample)
		{
			// resize color texture
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rgbaTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, this->width, this->height, GL_TRUE);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

			// resize depth attachment
			glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, this->width, this->height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			//resize depth texture
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT24, this->width, this->height, GL_TRUE);
		}
		else
		{
			// resize color texture
			glBindTexture(GL_TEXTURE_2D, rgbaTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glBindTexture(GL_TEXTURE_2D, 0);

			// resize depth attachment
			glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			//resize depth texture
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, this->width, this->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		}

}

void FrameBuffer::blit(std::unique_ptr<FrameBuffer> const & targetFramebuffer)
{
	if (targetFramebuffer == nullptr)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, this->frameBufferID);
		glBlitFramebuffer(0, 0, this->width, this->height,
			0, 0, this->width, this->height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glDrawBuffer(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFramebuffer->frameBufferID);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, this->frameBufferID);
		glBlitFramebuffer(0, 0, this->width, this->height,
			0, 0, targetFramebuffer->width, targetFramebuffer->height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glDrawBuffer(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
