#pragma once

#include <glm/glm.hpp>

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
** This class serves as the base class for any object whose state is tracked by
** the server. This includes players, walls, lights, dog bones, etc. Anything
** whose state relies on the server must be derived from this class.
*/
class GameEntity
{
private:
	uint32_t _id;      // Object ID
	glm::vec3 _pos;	   // World-coord position of object
	glm::vec3 _up;	   // Up vector of object
	glm::vec3 _normal; // Normal vector of object
	double _scale;	   // Scale of object

public:
	GameEntity();
	GameEntity(uint32_t id,
			   glm::vec3 pos,
			   glm::vec3 up,
			   glm::vec3 normal,
			   double scale)
	{
		_id = id;
		_pos = pos;
		_up = up;
		_normal = normal;
		_scale = scale;
	};
	~GameEntity();

	// Serialization for Cereal
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(_id,
				_pos,
				_up,
				_normal,
				_scale);
	};

	// Every class deriving this one must implement this function. Type
	// definitions go in Shared/Common.hpp. This is going to be one of the
	// necessary evils in the project, unfortunately.
	virtual EntityType getType() { return ENTITY_BASE; }

	// Must be implemented by all children. Not really needed on the server,
	// but this makes client-side stuff a bit easier
	virtual void updateState(GameEntity & entity)
	{
		_pos = entity.getPosVec();
		_up = entity.getUpVec();
		_normal = entity.getNormalVec();
		_scale = entity.getScale();
	};

	// Getters for private vars
	uint32_t getId() { return _id; }
	glm::vec3 getPosVec() { return _pos; }
	glm::vec3 getUpVec() { return _up; }
	glm::vec3 getNormalVec() { return _normal; }
	double getScale() { return _scale; }

	// Setters for private vars
	void setPosVec(glm::vec3 pos) { _pos = pos; }
	void setUpVec(glm::vec3 up) { _up = up; }
	void setNormalVec(glm::vec3 normal) { _normal = normal; }
	void setScale(double scale) { _scale = scale; }
};

