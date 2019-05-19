#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <queue>
#include <vector>
#include <unordered_map>

struct StructureInfo
{
	std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* entityMap;
	std::vector<std::shared_ptr<SBaseEntity>>* newEntities;
	std::vector<glm::vec2>* jails;
	std::queue<glm::vec2>* humanSpawns;
	std::queue<glm::vec2>* dogSpawns;
	std::vector<std::shared_ptr<SBaseEntity>>* dogHouses;
};
