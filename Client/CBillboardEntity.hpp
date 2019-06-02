#pragma once
#include "CBaseEntity.hpp"
#include "Font.h"
#include "Model.hpp"

class CBillboardEntity : public CBaseEntity
{
public:
	CBillboardEntity()
	{
		_objectModel = std::make_unique<Model>("./Resources/Models/cube.fbx");
		_objectShader = std::make_unique<Shader>();

		// Same base info as the object it is attached to
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();

		_text = std::make_unique<Font>();
		_text->_textColor = glm::vec4(1, 1, 1, 0.5);
	}

	~CBillboardEntity() {};

	void render(std::unique_ptr<Camera> const& camera) override
	{
		_objectShader->Use();
		
		CBaseEntity::setUniforms(camera);

		// Uniforms
        glm::vec3 pos = _state->pos;
		pos.y += _state->height;
        glm::vec3 forward = camera->position() - _state->pos;
		const auto t = glm::translate(glm::mat4(1.0f), pos);
		const auto r = glm::mat4(glm::transpose(glm::mat3(camera->view_matrix())));
		const auto s = glm::scale(glm::mat4(1.0f), _state->scale);

		auto model = t * r * s;
		_objectShader->set_uniform("u_model", model);

		// Render text
        _text->display(false, camera, model, _string.c_str(), 2);

		// TODO: only create text texture, then render the cube with it

		// Render billboard with texture (not working)
		//Mesh cubeMesh = (static_cast<Model*>(_objectModel.get()))->getMeshAt(0);
		//cubeMesh.Draw(_objectShader, _text->getTexture());
	}

	void setText(std::string string)
	{
		_string = string;
	}

private:
	std::string _string;
	std::unique_ptr<Font> _text;
};
