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

		_children.push_back(std::make_shared<SBoxPlungerEntity>(pos, scale));
	};
	~SFenceEntity() {};

	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _children;
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _children;
};

