#pragma once
#include "CBaseEntity.hpp"
#include "Font.h"
#include "Model.hpp"
#include "GUIManager.hpp"

class CBillboardEntity : public CBaseEntity
{
public:
	CBillboardEntity()
	{
		_objectModel = std::make_unique<Model>("Resources/Models/billboardcube.obj");
		_objectShader = std::make_unique<Shader>();

		// Same base info as the object it is attached to
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();

		_text = std::make_unique<Font>();
		_text->_textColor = glm::vec4(1, 1, 1, 1);
	}

	~CBillboardEntity() {};

	void render(std::unique_ptr<Camera> const& camera) override
	{
		// Save previous framebuffer
		GLint oldFBO;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldFBO);

		// Render text with its own framebuffer
		auto screen = GuiManager::getInstance().getScreen();
		_text->setScreenSize(glm::ivec2(screen->size().x(), screen->size().y()));
		_text->_backgroundColor = glm::vec4(1, 1, 1, 0.0f);
        _text->cut = true;
		_text->renderToTexture(_string.c_str(), 2.5f);
        _text->edge = glm::vec2(0.03,0.03);

		// Bind previous framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

		_objectShader->Use();
		_state->scale.z = 0.001f;
		_state->scale.y = 1.0f;
		
		CBaseEntity::setUniforms(camera);

		// Uniforms
        glm::vec3 pos = _state->pos;
		pos.y += _state->height + 0.25f;
        glm::vec3 forward = camera->position() - _state->pos;
		const auto t = glm::translate(glm::mat4(1.0f), pos);
		const auto r = glm::mat4(glm::transpose(glm::mat3(camera->view_matrix())));
		const auto s = glm::scale(glm::mat4(1.0f), _state->scale);

		auto model = t * r * s;
		_objectShader->set_uniform("u_model", model);

		// Render billboard with texture
		Mesh cubeMesh = (static_cast<Model*>(_objectModel.get()))->getMeshAt(0);
		cubeMesh.Draw(_objectShader, _text->getTexture());
	}

	void setText(std::string string)
	{
		_string = string;
	}

private:
	std::string _string;
	std::unique_ptr<Font> _text;

	std::vector<Vertex> _billboardVertices;
};
