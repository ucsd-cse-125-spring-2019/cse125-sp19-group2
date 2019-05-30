#pragma once

#include "SBoxEntity.hpp"
#include "SBoxPlungerEntity.hpp"

class SFenceEntity : public SBoxEntity
{
public:
	SFenceEntity(
		glm::vec3 pos,
		glm::vec3 scale) : SBoxEntity(pos, scale)
	{
		_state->type = ENTITY_FENCE;
		_state->isVisible = true;
		_children.push_back(std::make_shared<SBoxPlungerEntity>(pos, scale));
	};

	~SFenceEntity()
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

