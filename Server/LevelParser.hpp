#pragma once

#include <vector>
#include "SBaseEntity.hpp"

/**
  * Interface to manage level parsing given a file
  */
class LevelParser
{
public:
	virtual std::vector<std::shared_ptr<SBaseEntity>> parseLevelFromFile(std::string path) = 0;
};

