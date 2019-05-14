#include <unordered_set>
#include <random>
#include "CollisionManager.hpp"
#include "SDogEntity.hpp"

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

CollisionManager::CollisionManager(
	std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>* entityMap)
{
	_entityMap = entityMap;
}

CollisionManager::~CollisionManager()
{
}

void CollisionManager::handleCollisions()
{
	// Build quadtree
	auto tree = new QuadTree({ glm::vec2(0), MAP_WIDTH / 2 });
	for (auto& entityPair : *_entityMap)
	{
		// Only insert if it has a collider
		auto entityState = entityPair.second->getState().get();
		if (entityState->colliderType != COLLIDER_NONE)
		{
			tree->insert(entityState);
		}
	}

	auto collisionSet = std::unordered_set<std::pair<BaseState*, BaseState*>, PairHash>();

	// Build set of pairs of collisions
	for (auto& entityPair : *_entityMap)
	{
		auto entity = entityPair.second;

		// Only run collision check if not static
		if (!entity->getState()->isStatic)
		{
			for (auto& collidingEntity : entity->getColliding(*tree))
			{
				collisionSet.insert({ entity->getState().get(), collidingEntity });
			}
		}
	}

	// Iterate until no collisions
	while (!collisionSet.empty())
	{
		auto collisionPair = (collisionSet.begin());
		auto objectA = collisionPair->first;
		auto objectB = collisionPair->second;

		// Erase from beginning
		collisionSet.erase(collisionPair);

		// Find A and B entities
		auto entityA = _entityMap->find(objectA->id)->second;
		auto entityB = _entityMap->find(objectB->id)->second;

		// Collision resolution on A & B
		entityA->handleCollision(entityB);

		// Rare but in some cases (like dog caught), we need to run resolution
		// on B as well
		if (entityA->isColliding(objectB))
		{
			entityB->handleCollision(entityA);
		}

		// Remove duplicates (e.g. <A,B> vs <B,A> if both players)
		if (!objectB->isStatic)
		{
			collisionSet.erase({ objectB, objectA });

			// Mark as changed
			entityB->hasChanged = true;
		}

		entityA->hasChanged = true;

		// Re-check for colliding
		for (auto& collidingEntity : entityA->getColliding(*tree))
		{
			// Only re-add if solid
			if (objectA->isSolid && collidingEntity->isSolid)
			{
				collisionSet.insert({ entityA->getState().get(), collidingEntity });
			}
		}
	}

	delete tree;
}