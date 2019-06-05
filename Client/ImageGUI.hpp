#pragma once

#include "Model.hpp"
#include "Shader.hpp"
#include "FrameBuffer.h"

class ImageGUI
{
protected:
	std::unique_ptr<Shader> _quadShader;
	std::unique_ptr<FrameBuffer> _quadFrameBuffer;
	unsigned int textureID;

	glm::vec2 _quad_size;
	glm::vec2 _win_size;
	glm::vec2 _quad_pos;
	float _quad_rotation;

public:

	virtual void init(float width, float height) {
		_quadShader = std::make_unique<Shader>();

		_quadShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/imageGUI.vert");
		_quadShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/imageGUI.frag");
		_quadShader->CreateProgram();

		updateWindowSize(width, height);
		_quad_rotation = 0.0f;

		_quad_size = glm::vec2(width, height);
		_quadFrameBuffer = std::make_unique<FrameBuffer>();
		_quad_pos = glm::vec2(0);
	}

	virtual void updateWindowSize(float width, float height) {
		_win_size = glm::vec2(width, height);
	}

	virtual void updateRotation(float degree) {
		_quad_rotation = degree;
	}

	virtual void render() {
		_quadShader->Use();

		// Compute model matrix based on state: t * r * s
		const auto t = glm::translate(glm::mat4(1.0f), glm::vec3((_quad_pos / _win_size * 2.0f) - 1.0f, 0));
		const auto r = glm::rotate(glm::mat4(1.0f), _quad_rotation, glm::vec3(0, 0, 1));
		const auto s = glm::scale(glm::mat4(1.0f), glm::vec3(_quad_size / _win_size, 1));

		auto model = t * s * r;

		// Pass model matrix into shader
		_quadShader->set_uniform("u_model", model);

		_quadFrameBuffer->drawQuad(_quadShader, textureID);
	}
};

