#pragma once

#include <glm/glm.hpp>
#include <string>
#include <iostream>

#include "Shared/Common.hpp"	// GameEntity type enum

// Allow Cereal serialization of GLM vectors
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
	double width;		// Width of object in X-axis
	double depth;		// Depth of object in Z-axis
	double height;		// Height of object in Y-axis

	// Object lifetime info
	bool isDestroyed;	// Object has been deleted on the server

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
				isDestroyed);
	};

	virtual void print()
	{
		std::cerr << "[type: " << type << ", id: " << id << ", scale: "
			<< "]" << std::endl;
	}

  // This is silly but it is required for Cereal to recognize this as a
  // polymorphic type. You do not have to implement or call this function.
  virtual uint32_t getId() { return id; }
};


