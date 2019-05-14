#include "EntityManager.hpp"
#include "CDogEntity.hpp"
#include "CHumanEntity.hpp"
#include "CBoxEntity.hpp"
#include "CHouseEntity.hpp"
#include "CBoneEntity.hpp"
#include "CDogHouseEntity.hpp"
#include "CHydrantEntity.hpp"
#include "CFountainEntity.hpp"
#include "CFloorEntity.hpp"
#include "ColliderManager.hpp"
#include "Shared/Logger.hpp"
#include <algorithm>

EntityManager::EntityManager() {
}

EntityManager& EntityManager::getInstance() {
    static EntityManager entityManager;
    return entityManager;
}

std::shared_ptr<CBaseEntity> EntityManager::getEntity(uint32_t id) {
    auto result = _entityMap.find(id);

    // Return if found
    if (result != _entityMap.end()) {
        return _entityList[result->second];
    }
    return nullptr;
}

std::shared_ptr<CBaseEntity> EntityManager::getEntity(std::shared_ptr<BaseState> const& state) {
    // Check if the Entity is already created
    uint32_t id = state->id;
    auto result = _entityMap.find(id);

    // Return if found
    if (result != _entityMap.end()) {
        return _entityList[result->second];
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
	case ENTITY_BONE:
		entity = std::make_shared<CBoneEntity>();
		break;
	case ENTITY_DOGHOUSE:
		entity = std::make_shared<CDogHouseEntity>();
		break;
	case ENTITY_HYDRANT:
		entity = std::make_shared<CHydrantEntity>();
		break;
	case ENTITY_FOUNTAIN:
		entity = std::make_shared<CFountainEntity>();
		break;
	case ENTITY_FLOOR:
		CFloorEntity::getInstance().updateTile(state);
		break;
    }

	if (entity)
	{
        _entityList.push_back(entity);
        _entityMap.insert({id, _entityList.size() - 1});
	}

    return entity;
}

void EntityManager::update(std::shared_ptr<BaseState> const& state) {
    // First check if marked as destroyed
    if (state->isDestroyed) {
        // Find in map and destroy if it exists
        auto result = _entityMap.find(state->id);
        if (result != _entityMap.end()) {
            _entityMap.erase(result);
        }

		ColliderManager::getInstance().erase(state->id);

        return;
    }

    auto entity = getEntity(state);

	if (entity)
	{
		entity->updateState(state);
	}

	ColliderManager::getInstance().updateState(state);
}

void EntityManager::render(std::unique_ptr<Camera> const& camera) {
    std::vector<bool> test(_entityList.size());

    for(uint32_t i = 0 ; i < _entityList.size(); i ++) {
        const glm::vec3 pos = _entityList[i]->getPos();
        const float radius = _entityList[i]->getRadius();
        test[i] = camera->isInFrustum(pos, radius);
    }

    for(uint32_t i = 0 ; i < _entityList.size(); i ++) {
        if(test[i]) {
            _entityList[i]->render(camera);
        }
    }
}
