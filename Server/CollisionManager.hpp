#pragma once

#include <memory>
#include <unordered_map>
#include "SBaseEntity.hpp"

/**
  * Basic class to handle collisions between all entities on the server
  */
class CollisionManager
{
public:
	// Constructs a new manager. Requires raw pointers to the server-wide
	// entity map and the vector of jail locations.
	CollisionManager(
		std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* entityMap,
		std::vector<glm::vec2>* jails);

	~CollisionManager();

	// Iterates over every non-static entity on the server and resolves
	// collisions on a pair-wise basis
	void handleCollisions();

private:
	std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* _entityMap;
	std::vector<glm::vec2>* _jails;
};

