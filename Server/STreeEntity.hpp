#pragma once

#include "SBaseEntity.hpp"
#include "CapsuleCollider.hpp"

class STreeEntity : public SBaseEntity
{
public:
	STreeEntity(glm::vec3 pos)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();

		_state->type = ENTITY_TREE;
		_state->pos = pos;

		// Capsule collider
		_collider = std::make_unique<CapsuleCollider>(_state.get());
		_state->colliderType = COLLIDER_CAPSULE;
		_state->width = 1.5f;
		_state->depth = 1.5f;
		_state->height = 2.3f;

		// Plunger collider
		_children.push_back(std::make_shared<SCylinderPlungerEntity>(
			pos, glm::vec3(1.3f, 2.3f, 1.3f)));
	}

	~STreeEntity()
	{
		_children.clear();
	};

	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _children;
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _children;
};
