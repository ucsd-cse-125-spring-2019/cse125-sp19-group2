/**
 * FrameBuffer.h
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include "Shader.hpp"
#include <memory>

/**
 * \brief A wrapper object that handles creation of framebuffer and rendering inside the framebuffer
 */
class FrameBuffer {
public:

    /**
     * \brief Default constructor. Create a FrameBuffer object with 1024x1024 texture
     */
    FrameBuffer();

    /**
     * \brief Constructor. Create a FrameBuffer object with specific size of texture.
     * \param width(int) Width of the RGBA and Depth texture
     * \param height(int) Height of the RGBA and Depth texture
     */
    FrameBuffer(int width, int height, bool multisample = false);

    ~FrameBuffer();

    /**
     * \brief Render scene inside this framebuffer. The RGBA result will go to 
     * rgbaTexture and depth result will go to depthTexture
     * \tparam T 
     * \param lambda(T&&) A function that renders the scene. It can include any render functions of other object.
     */
    template <typename T>
    void renderScene(T&& lambda);

    /**
     * \brief Render rgbaTexture content onto a fullscreen(depends on viewport size) quad in the 
     * current framebuffer: it can be either onto other FrameBuffer object or onto the default 
     * framebuffer(onto screen directly)
     * \param quadShader(std::unique_ptr<Shader>const&) The shader program to render this quad.
     * \param externalTexture(unsigned) If anything other than -1 is passed in, then it will try to use the input texture
     * to render the quad.
     */
    void drawQuad(std::unique_ptr<Shader> const& quadShader, unsigned int externalTexture = -1);

    /**
     * \brief Getter of rgbaTexture
     * \return GLuint: TextureID of rgbaTexture
     */
    GLuint getRGBA();

    /**
     * \brief Getter of depthTexture
     * \return GLuint: TextureID of depthTexture
     */
    GLuint getDepth();

    /**
     * \brief Function needs to be called when the window resizes
     * \param x(int) Width of the window
     * \param y(int) Height of the window
     */
    void resize(int x, int y);

		/**
		 * \brief Copies a block of pixels from this framebuffer to a target framebuffer.
		 * \param targetFramebuffer(std::unique_ptr<FrameBuffer>const&) Target framebuffer to copy data to.
		 */
		void blit(std::unique_ptr<FrameBuffer> const &targetFramebuffer = nullptr);

private:
    void setEnable(bool status);

    GLuint frameBufferID, depthBufferID;
    GLuint rgbaTexture, depthTexture;
    GLuint VAO, VBO;
    GLuint width, height;

		bool multisample;
};

template <typename T>
void FrameBuffer::renderScene(T&& lambda) {
    setEnable(true);
    lambda();
    setEnable(false);
}
#endif
