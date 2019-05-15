#pragma once

#include "SBaseEntity.hpp"
#include "EmptyCollider.hpp"

// Special entity that is used on the client-side to construct the ground
// Represents a single tile
class SFloorEntity : public SBaseEntity
{
public:
	SFloorEntity(
		FloorType type,
		int xIndex,
		int zIndex,
		float scale)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_FLOOR;

		// Save scale of tile
		_state->width = scale;
		_state->depth = scale;

		// Encode floor type in height variable
		_state->height = (float)type;

		// Position is saved with indices
		_state->pos.x = (float)xIndex;
		_state->pos.z = (float)zIndex;

		// Floor tiles don't collide with anything
		_collider = std::make_unique<EmptyCollider>();
		_state->colliderType = COLLIDER_NONE;
	};
	~SFloorEntity() {};
};

