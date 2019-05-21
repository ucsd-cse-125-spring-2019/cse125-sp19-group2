#pragma once

#include <vector>
#include "SBaseEntity.hpp"
#include "StructureInfo.hpp"

/**
  * Interface to manage level parsing given a file
  */
class LevelParser
{
public:
	// Jail locations, human locations, and dog spawns are empty queues passed
	// to the function, and parsed positions are added to them. Queues are used
	// because it is easy to shuffle through each element in the container.
	virtual void parseLevelFromFile(
		std::string path,
		StructureInfo* structureInfo) = 0;
};

