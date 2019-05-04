#pragma once

#include <memory>
#include <unordered_map>
#include "SBaseEntity.hpp"

// Allow pairs inside unordered_set
struct PairHash
{
	template <class T1, class T2>
	std::size_t operator () (std::pair<T1, T2> const &pair) const
	{
		std::size_t h1 = std::hash<T1>()(pair.first);
		std::size_t h2 = std::hash<T2>()(pair.second);

		return h1 ^ h2;
	}
};

class CollisionManager
{
public:
	CollisionManager(
		std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* entityMap,
		std::vector<glm::vec2>* jails);

	~CollisionManager();

	void handleCollisions();

	// Called in SBaseEntity handleCollision()
	static void handleAABB(BaseState* stateA, BaseState* stateB);
	static void handleCapsule(BaseState* stateA, BaseState* stateB);

private:
	std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* _entityMap;
	std::vector<glm::vec2>* _jails;

	void handleDogCaught(
		BaseState* dog,
		BaseState* human,
		std::unordered_set<std::pair<BaseState*, BaseState*>, PairHash> & collisionSet,
		QuadTree * tre);
};

