#include "EntityManager.hpp"
#include "CPlayerEntity.hpp"

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
    if(result != entityList.end()) {
		return result->second;
    }

    // Otherwise create the Entity based on its type
	std::shared_ptr<CBaseEntity> entity;
	EntityType type = state->type;
    switch(type) {
	case ENTITY_EXAMPLE:
		break;
	case ENTITY_PLAYER:
		entity = std::make_shared<CPlayerEntity>();
		break;
    }
	entityList.insert({id, entity});
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
    entity->updateState(state);
}
