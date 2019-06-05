#pragma once
#include "ImageGUI.hpp"

class DogPointerGUI : public ImageGUI
{
protected:
	unsigned int arrowTextureID;
	unsigned int iconTextureID;
	float radius;

public:

	DogPointerGUI(float width, float height) {
		ImageGUI::init(width, height);
		_quad_size = glm::vec2(DOGPOINTER_SIZE, DOGPOINTER_SIZE);
		arrowTextureID = LoadTextureFromFile("arrow.png", "./Resources/Textures/Menu");
	};

	virtual void updateWindowSize(float width, float height) override {
		ImageGUI::updateWindowSize(width, height);
		float actualRadius = std::min(width, height) * DOGPOINTER_RADIUS_RATIO;
		radius = actualRadius / DOGPOINTER_SIZE;
	}

	virtual void render() override {
		_quadShader->Use();

		// Compute model matrix for compass
		const auto t = glm::translate(glm::mat4(1.0f), glm::vec3(0, radius, 0));
		const auto r = glm::rotate(glm::mat4(1.0f), _quad_rotation, glm::vec3(0, 0, 1));
		const auto s = glm::scale(glm::mat4(1.0f), glm::vec3(_quad_size / _win_size, 1));

		auto model = s * r * t;

		// Pass model matrix into shader
		_quadShader->set_uniform("u_model", model);

		_quadFrameBuffer->drawQuad(_quadShader, arrowTextureID);

		// Compute model matrix for pointer
		//model = t * s;

		// Pass model matrix into shader
		//_quadShader->set_uniform("u_model", model);

		//_quadFrameBuffer->drawQuad(_quadShader, pointerTextureID);
	}
};

