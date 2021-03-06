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
	virtual void render(std::unique_ptr<Camera> const &camera)
	{
		_objectShader->Use();
		setUniforms(camera);
		_objectModel->render(_objectShader);
	}

	virtual void setUniforms(std::unique_ptr<Camera> const &camera)
	{
		_objectShader->set_uniform("u_projection", camera->projection_matrix());
		_objectShader->set_uniform("u_view", camera->view_matrix());

		// Setting tranparency
		_objectShader->set_uniform("u_transparency", _state->transparency * _alpha);

		// Compute model matrix based on state: t * r * s
		const auto t = glm::translate(glm::mat4(1.0f), _state->pos);
		const auto r = glm::lookAt(glm::vec3(0.0f), _state->forward, _state->up);
		const auto s = glm::scale(glm::mat4(1.0f), _state->scale);

		auto model = t * r * s;

		// Pass model matrix into shader
		_objectShader->set_uniform("u_model", model);
		_objectShader->set_uniform("u_dirlight.direction", glm::vec3(-0.4f, -1.0f, -0.4f));
		_objectShader->set_uniform("u_dirlight.ambient", glm::vec3(0.2f, 0.2f, 0.3f));
		_objectShader->set_uniform("u_dirlight.diffuse", glm::vec3(0.8f, 0.8f, 0.9f));
		_objectShader->set_uniform("u_numdirlights", static_cast<GLuint>(1));
	}

	// Every child must override this if they carry additional state
	virtual void updateState(std::shared_ptr<BaseState> state)
	{
		_state->id = state->id;

		// Translation
		_state->pos = state->pos;

		// Orientation
		_state->forward = state->forward;
		_state->up = state->up;

		// Scale
		_state->scale = state->scale;

		// Dimensions for bounding boxes
		_state->width = state->width;
		_state->height = state->height;
		_state->depth = state->depth;
		_state->transparency = state->transparency;
		_state->type = state->type;
	}

	// This is optional, but might make our lives easier. Remove if you feel
	// that it is not necessary
	uint32_t getId()
	{
		return _state->id;
	}

	EntityType getType()
	{
		return _state->type;
	}

	// Getter for culling
	virtual glm::vec3 getPos() const
	{
		return _state->pos;
	}

	virtual float getRadius() const
	{
		const auto scale = _state->scale;
		return std::max(_state->width, std::max(_state->height, _state->depth));
	}

	// TODO: add any functionality here that is needed in every client-side
	// object.

	virtual void setAlpha(float transparency)
	{
		_alpha = transparency;
	}

	virtual float getAlpha() const
	{
		return _alpha * _state->transparency;
	}

protected:
	// State pointer for object
	std::shared_ptr<BaseState> _state;

	// Drawable object. Can be an animation or model
	std::unique_ptr<Drawable> _objectModel;

	// Transparency information
	float _alpha = 1.0f;

	// Shader program for object
	std::unique_ptr<Shader> _objectShader;
};
