#pragma once
#include "ImageGUI.hpp"

class TooltipGUI : public ImageGUI
{
public:
	TooltipGUI(float width, float height)
	{
		ImageGUI::init(width, height);
		_quad_size = glm::vec2(TOOLTIP_SIZE, TOOLTIP_SIZE);
		_fountainTooltipTexture = LoadTextureFromFile("drink_instructions.png", "Resources/Textures/Menu/");
		_jailTooltipTexture = LoadTextureFromFile("jail_instructions.png", "Resources/Textures/Menu/");
	}

	virtual void updateWindowSize(float width, float height) override
	{
		ImageGUI::updateWindowSize(width, height);
		_quad_pos = glm::vec2(width / 2 + TOOLTIP_OFFSET, height / 2 + TOOLTIP_OFFSET);
	}

	virtual void render() override {
		_quadShader->Use();

		// Compute model matrix for compass
		const auto t = glm::translate(glm::mat4(1.0f), glm::vec3((_quad_pos / _win_size * 2.0f) - 1.0f, 0));
		const auto s = glm::scale(glm::mat4(1.0f), glm::vec3(_quad_size / _win_size, 1));

		auto model = t * s;

		// Pass model matrix into shader
		_quadShader->set_uniform("u_model", model);
		_quadShader->set_uniform("alpha", 0.8f);

		switch (_currentTooltip)
		{
		case TOOLTIP_JAIL:
			_quadFrameBuffer->drawQuad(_quadShader, _jailTooltipTexture);
			break;
		case TOOLTIP_DRINK:
			_quadFrameBuffer->drawQuad(_quadShader, _fountainTooltipTexture);
			break;
		}
	}

	void setTooltip(PlayerTooltip tooltip)
	{
		_currentTooltip = tooltip;
	}

private:
	PlayerTooltip _currentTooltip = TOOLTIP_NONE;
	GLuint _fountainTooltipTexture;
	GLuint _jailTooltipTexture;
};