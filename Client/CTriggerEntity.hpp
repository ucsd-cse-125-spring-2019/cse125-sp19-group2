#pragma once
#include "CBaseEntity.hpp"
#include "Model.hpp"

class CTriggerEntity : public CBaseEntity
{
public:
	CTriggerEntity()
	{
		// Allocate member variables
		_objectModel = std::make_unique<Model>("./Resources/Models/gate_trigger.fbx");
		_objectShader = std::make_unique<Shader>();
		_state = std::make_shared<BaseState>();

		_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
		_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
		_objectShader->CreateProgram();
	};
	~CTriggerEntity() {};

	void setUniforms(std::unique_ptr<Camera> const& camera) override
	{
		CBaseEntity::setUniforms(camera);

		// Compute model matrix based on state: t * r
		const auto t = glm::translate(glm::mat4(1.0f), _state->pos);
		const auto r = glm::lookAt(glm::vec3(0.0f), _state->forward, _state->up);

		// magnitude of forward - 1 is the amount of degree to rotate
		float rotateD = abs(_state->forward.x + _state->forward.z) - 1;

		auto r1 = glm::rotate(glm::radians(rotateD), glm::vec3(-1,0,0));

		auto model = t * r * r1;

		// Pass model matrix into shader
		_objectShader->set_uniform("u_model", model);
	}
};
