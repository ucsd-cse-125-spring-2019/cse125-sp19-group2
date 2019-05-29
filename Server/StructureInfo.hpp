#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <queue>
#include <vector>
#include <unordered_map>
#include "Shared/GameState.hpp"

class SJailEntity;

struct StructureInfo
{
	std::shared_ptr<GameState> gameState = nullptr;
	std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* entityMap = nullptr;
	std::vector<std::shared_ptr<SBaseEntity>>* newEntities = nullptr;
	std::vector<glm::vec2>* jailsPos = nullptr;
	std::queue<glm::vec2>* humanSpawns = nullptr;
	std::queue<glm::vec2>* dogSpawns = nullptr;
	std::vector<std::shared_ptr<SBaseEntity>>* dogHouses = nullptr;
	std::vector<std::shared_ptr<SJailEntity>>* jails = nullptr;
};
