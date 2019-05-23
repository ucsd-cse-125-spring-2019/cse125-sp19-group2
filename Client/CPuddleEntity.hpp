#pragma once
#include "CBaseEntity.hpp"
#include "Model.hpp"
#include "Shared/Logger.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// As with server, TONS of copied code. Need to refactor static objects
// to initialize all this junk in a parent class.
class CPuddleEntity : public CBaseEntity
{
public:
	CPuddleEntity() {
		// Allocate member variables
		_objectModel = std::make_unique<Model>("./Resources/Models/pee.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};

	~CPuddleEntity() {};
	
	void setUniforms(std::unique_ptr<Camera> const& camera) override
	{
		CBaseEntity::setUniforms(camera);

		// Transparency (don't use alpha)
		_objectShader->set_uniform("u_transparency", _state->transparency);
	}
};
