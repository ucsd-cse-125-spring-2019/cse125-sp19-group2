#pragma once

#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <functional>

#include "Shared/Common.hpp"	// GameEntity type enum

// Allow Cereal serialization of GLM 3-item vectors
namespace cereal
{
	template<class Archive>
	void serialize(Archive & archive, glm::vec3 &v)
	{
        archive(v.x, v.y, v.z);
	}
}

/*
** This struct serves as the base for any object whose state is tracked by
** the server. This includes players, walls, lights, dog bones, etc. It can be
** extended as necessary. Ensure that all children have proper serialization
** functions as well.
**
** E.g.
** struct PlayerState : BaseState {}
** struct LightState : BaseState {}
** etc.
*/
struct BaseState
{
	// General object state
    EntityType type;	// Type of object. Nasty but necessary for object creation
	uint32_t id;		// Object ID

	// Spatial information
	glm::vec3 pos;		// World-coord position of object
	glm::vec3 up;		// Up vector of object
	glm::vec3 forward;	// Forward vector of object
	glm::vec3 scale;	// Scale of object

	// Bounding box stuff
	float width;		// Width of object in X-axis
	float depth;		// Depth of object in Z-axis
	float height;		// Height of object in Y-axis
	ColliderType colliderType;

	// Transparency information
	float transparency;

	// Object lifetime info
	bool isDestroyed;	// Object has been deleted on the server

	// Whether or not the object can move
	bool isStatic;

	// Whether or not the object can be passed through
	bool isSolid;

	// Self-explanatory
	bool isVisible;

	// Serialization for Cereal
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(type,
				id,
				pos,
				up,
				forward,
				scale,
				width,
				depth,
				height,
				colliderType,
				transparency,
				isDestroyed,
				isStatic,
				isSolid,
				isVisible);
	};

	bool getSolidity(BaseState* entity)
	{
		if (solidFunc == 0)
		{
			return isSolid;
		}
		else
		{
			return solidFunc(this, entity);
		}
	}

	void setSolidity(std::function<bool(BaseState* entity, BaseState* collidingEntity)> f)
	{
		solidFunc = f;
	}

	std::function<bool(BaseState* entity, BaseState* collidingEntity)> solidFunc = 0;



	virtual void print()
	{
		std::cerr << "[type: " << type << ", id: " << id << ", scale: "
			<< "]" << std::endl;
	}

  // This is silly but it is required for Cereal to recognize this as a
  // polymorphic type. You do not have to implement or call this function.
  virtual uint32_t getId() { return id; }
};


