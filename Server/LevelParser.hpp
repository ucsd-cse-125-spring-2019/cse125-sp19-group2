#pragma once

#include <vector>
#include "SBaseEntity.hpp"

/**
  * Interface to manage level parsing given a file
  */
class LevelParser
{
public:
	// Jail locations, human locations, and dog spawns are empty queues passed
	// to the function, and parsed positions are added to them. Queues are used
	// because it is easy to shuffle through each element in the container.
	virtual std::vector<std::shared_ptr<SBaseEntity>> parseLevelFromFile(
		std::string path,
		std::vector<glm::vec2> & jailLocations,
		std::queue<glm::vec2> & humanSpawns,
		std::queue<glm::vec2> & dogSpawns) = 0;
};

