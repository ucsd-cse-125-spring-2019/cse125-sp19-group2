#pragma once

#include <memory>

#include "Shared/BaseState.hpp"
#include "Camera.hpp"
#include "Drawable.hpp"
#include "Shader.hpp"

/*
** This is an interface that any graphics objects on the client side must be
** derived from. Do not instantiate objects of this class.
**
** E.g.
** class CDogBone : public CBaseEntity {..}
** class CPlayer : public CBaseEntity {..}
** etc.
*/
class CBaseEntity
{
public:
	// Base render function, affects all entities. If special handling is
	// needed, override this in the child.
	virtual void render(std::unique_ptr<Camera> const& camera)
	{
		_objectShader->Use();
		_objectShader->set_uniform("u_projection", camera->projection_matrix());
		_objectShader->set_uniform("u_view", camera->view_matrix());

		// Compute model matrix based on state: t * r * s
		const auto t = glm::translate(glm::mat4(1.0f), _state->pos);
		const auto r = glm::lookAt(glm::vec3(0.0f), _state->forward, _state->up);
		const auto s = glm::scale(glm::mat4(1.0f), _state->scale);

		auto model = t * r * s;

		// Pass model matrix into shader
		_objectShader->set_uniform("u_scale", s);
		_objectShader->set_uniform("u_model", model);
		_objectShader->set_uniform("u_material.shininess", 0.6f);
		_objectShader->set_uniform("u_pointlight.position", glm::vec3(-3.0f, 3.0f, -3.0f));
		_objectShader->set_uniform("u_pointlight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		_objectShader->set_uniform("u_pointlight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		_objectShader->set_uniform("u_pointlight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		_objectShader->set_uniform("u_pointlight.constant", 1.0f);
		_objectShader->set_uniform("u_pointlight.linear", 0.09f);
		_objectShader->set_uniform("u_pointlight.quadratic", 0.032f);
		_objectShader->set_uniform("u_numpointlights", static_cast<GLuint>(1));
		_objectModel->render(_objectShader);
	}

    // Every child must override this if they carry additional state
	virtual void updateState(std::shared_ptr<BaseState> state)
	{
		// Translation
		_state->pos = state->pos;

		// Orientation
		_state->forward = state->forward;
		_state->up = state->up;

		// Scale
		_state->scale = state->scale;
	}

    // This is optional, but might make our lives easier. Remove if you feel
    // that it is not necessary
	uint32_t getId()
	{
		return _state->id;
	}

	// TODO: add any functionality here that is needed in every client-side
	// object.

protected:
	// State pointer for object
	std::shared_ptr<BaseState> _state;

	// Drawable object. Can be an animation or model
	std::unique_ptr<Drawable> _objectModel;

	// Shader program for object
	std::unique_ptr<Shader> _objectShader;
};

