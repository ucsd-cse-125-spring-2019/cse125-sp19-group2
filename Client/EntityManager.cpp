#include "EntityManager.hpp"
#include "CDogEntity.hpp"
#include "CHumanEntity.hpp"
#include "CBoxEntity.hpp"
#include "CHouseEntity.hpp"
#include "Shared/Logger.hpp"
#include <algorithm>

EntityManager::EntityManager() {
}

EntityManager& EntityManager::getInstance() {
    static EntityManager entityManager;
    return entityManager;
}

std::shared_ptr<CBaseEntity> EntityManager::getEntity(uint32_t id) {
    auto result = entityList.find(id);

    // Return if found
    if (result != entityList.end()) {
        return result->second;
    }
    return nullptr;
}

std::shared_ptr<CBaseEntity> EntityManager::getEntity(std::shared_ptr<BaseState> const& state) {
    // Check if the Entity is already created
    uint32_t id = state->id;
    auto result = entityList.find(id);

    // Return if found
    if (result != entityList.end()) {
        return result->second;
    }

    // Otherwise create the Entity based on its type
    std::shared_ptr<CBaseEntity> entity = nullptr;
    EntityType type = state->type;
    switch (type) {
    case ENTITY_EXAMPLE:
        break;
    case ENTITY_DOG:
        entity = std::make_shared<CDogEntity>();
        break;
	case ENTITY_HUMAN:
		entity = std::make_shared<CHumanEntity>();
		break;
	case ENTITY_BOX:
		entity = std::make_shared<CBoxEntity>();
		break;

	case ENTITY_HOUSE_6X6_A:
		entity = std::make_shared<CHouseEntity>(type);
		break;
    }

	if (entity)
	{
		entityList.insert({ id, entity });
	}

    return entity;
}

void EntityManager::update(std::shared_ptr<BaseState> const& state) {
    // First check if marked as destroyed
    if (state->isDestroyed) {
        // Find in map and destroy if it exists
        auto result = entityList.find(state->id);
        if (result != entityList.end()) {
            entityList.erase(result);
        }
        return;
    }

    auto entity = getEntity(state);

	if (entity)
	{
		entity->updateState(state);
	}
}

void EntityManager::render(std::unique_ptr<Camera> const& camera) {
    std::for_each(entityList.begin(), entityList.end(),
        [&camera](std::pair<uint32_t, std::shared_ptr<CBaseEntity>> entity)
        {
            entity.second->render(camera);
        });
}
