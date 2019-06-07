#pragma once
#include "ImageGUI.hpp"

class CompassGUI : public ImageGUI
{
protected:
	unsigned int compassTextureID;
	unsigned int pointerTextureID;

public:

	CompassGUI(float width, float height) {
		ImageGUI::init(width, height);
		_quad_size = glm::vec2(COMPASS_SIZE, COMPASS_SIZE);
		compassTextureID = LoadTextureFromFile("compass.png", "./Resources/Textures/Menu");
		pointerTextureID = LoadTextureFromFile("compassneedle.png", "./Resources/Textures/Menu");
	};

	virtual void updateWindowSize(float width, float height) override {
		ImageGUI::updateWindowSize(width, height);
		float rescaleFactor = (float)(width) / 1280;
		_quad_size = glm::vec2(COMPASS_SIZE * rescaleFactor, COMPASS_SIZE * rescaleFactor);
		_quad_pos = glm::vec2(width - COMPASS_POS_OFFSET * rescaleFactor, height - COMPASS_POS_OFFSET * rescaleFactor);
	}

	virtual void render() override {
		_quadShader->Use();

		// Compute model matrix for compass
		const auto t = glm::translate(glm::mat4(1.0f), glm::vec3((_quad_pos / _win_size * 2.0f) - 1.0f, 0));
		const auto r = glm::rotate(glm::mat4(1.0f), _quad_rotation, glm::vec3(0, 0, 1));
		const auto s = glm::scale(glm::mat4(1.0f), glm::vec3(_quad_size / _win_size, 1));

		auto model = t * s * r;

		// Pass model matrix into shader
		_quadShader->set_uniform("u_model", model);

		_quadFrameBuffer->drawQuad(_quadShader, compassTextureID);

		// Compute model matrix for pointer
		model = t * s;

		// Pass model matrix into shader
		_quadShader->set_uniform("u_model", model);

		_quadFrameBuffer->drawQuad(_quadShader, pointerTextureID);
	}
};

