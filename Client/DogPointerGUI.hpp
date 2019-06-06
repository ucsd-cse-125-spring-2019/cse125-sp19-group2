#pragma once
#include "ImageGUI.hpp"

class DogPointerGUI : public ImageGUI
{
protected:
	unsigned int arrowTextureID;
	unsigned int iconTextureID;
	float radius;
	float borderWidth = 0.8f;
	float borderHeight = 0.8f;

public:

	DogPointerGUI(float width, float height) {
		ImageGUI::init(width, height);
		_quad_size = glm::vec2(DOGPOINTER_SIZE, DOGPOINTER_SIZE);
		arrowTextureID = LoadTextureFromFile("arrow.png", "./Resources/Textures/Menu");
	};

	virtual void updateWindowSize(float width, float height) override {
		ImageGUI::updateWindowSize(width, height);
		//version 1 update
		float actualRadius = std::min(width, height) * DOGPOINTER_RADIUS_RATIO;
		radius = actualRadius / DOGPOINTER_SIZE;

		//version 2 update
		borderWidth = (width - DOGPOINTER_BORDER_OFFSET) / width;
		if (borderWidth < 0) borderWidth = 0.8f;
		borderHeight = (height - DOGPOINTER_BORDER_OFFSET) / height;
		if (borderHeight < 0) borderHeight = 0.8f;
	}
	
	// version 1 (rotate around radius) (similar to compass)
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
	}

	virtual void render(std::unique_ptr<Camera> const &camera, glm::vec3 targetPos) {
		_quadShader->Use();

		auto model_t = glm::translate(glm::mat4(1.0f), targetPos);
		auto clipSpacePos = camera->projection_matrix() * camera->view_matrix() * model_t * glm::vec4(0,0,0,1);
		glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;

		glm::vec2 screenPos = ndcSpacePos;
		// position dont need to change if target within frustum
		if (camera->isInFrustum(targetPos, 0)) {
			if (std::abs(screenPos.x) > borderWidth) {
				screenPos = screenPos / std::abs(screenPos.x) * borderWidth;
			}
			if (std::abs(screenPos.y) > borderHeight) {
				screenPos = screenPos / std::abs(screenPos.y)* borderHeight;
			}
		}
		// position need to push the edge if target is out of frustum
		else {
			if (ndcSpacePos.z > 1.0f) {
				screenPos *= -1;
			}
			// push to edge of screen if oppisite side of screen
			if (std::abs(screenPos.x) < borderWidth && std::abs(screenPos.y) < 0.8f) {
				screenPos /= -std::max(std::abs(screenPos.x), std::abs(screenPos.y));
			}
			// push back to screen if outside os screen

			if (std::abs(screenPos.x) > borderWidth) {
				screenPos = screenPos / std::abs(screenPos.x) * borderWidth;
			}
			if (std::abs(screenPos.y) > borderHeight) {
				screenPos = screenPos / std::abs(screenPos.y)* borderHeight;
			}
		}

		// Compute model matrix for compass
		const auto t = glm::translate(glm::mat4(1.0f), glm::vec3(screenPos, 0));
		//const auto r = glm::rotate(glm::mat4(1.0f), _quad_rotation, glm::vec3(0, 0, 1));
		const auto s = glm::scale(glm::mat4(1.0f), glm::vec3(_quad_size / _win_size, 1));

		auto model = t * s;

		// Pass model matrix into shader
		_quadShader->set_uniform("u_model", model);

		_quadFrameBuffer->drawQuad(_quadShader, arrowTextureID);
	}
};

