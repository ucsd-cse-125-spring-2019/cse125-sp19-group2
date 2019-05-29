#include "EntityManager.hpp"
#include "CDogEntity.hpp"
#include "CHumanEntity.hpp"
#include "CBoxEntity.hpp"
#include "CHouseEntity.hpp"
#include "CBoneEntity.hpp"
#include "CDogHouseEntity.hpp"
#include "CHydrantEntity.hpp"
#include "CFountainEntity.hpp"
#include "CPuddleEntity.hpp"
#include "CFloorEntity.hpp"
#include "CFenceEntity.hpp"
#include "CBarEntity.hpp"
#include "CGateEntity.hpp"
#include "CTriggerEntity.hpp"
#include "CPlungerEntity.hpp"
#include "CRopeEntity.hpp"
#include "CTrapEntity.hpp"
#include "CTreeEntity.hpp"
#include "ColliderManager.hpp"
#include "ParticleSystemManager.hpp"
#include "Shared/Logger.hpp"
#include <algorithm>

std::unique_ptr<Shader> CPlungerEntity::plungerShader;

EntityManager::EntityManager()
{
}

EntityManager &EntityManager::getInstance()
{
    static EntityManager entityManager;
    return entityManager;
}

std::shared_ptr<CBaseEntity> EntityManager::getEntity(uint32_t id)
{
    auto result = _entityMap.find(id);

    // Return if found
    if (result != _entityMap.end())
    {
        return _entityList[result->second];
    }
    return nullptr;
}

std::shared_ptr<CBaseEntity> EntityManager::getEntity(std::shared_ptr<BaseState> const &state)
{
    // Check if the Entity is already created
    uint32_t id = state->id;
    auto result = _entityMap.find(id);

    // Return if found
    if (result != _entityMap.end())
    {
        return _entityList[result->second];
    }

    // Otherwise create the Entity based on its type
    std::shared_ptr<CBaseEntity> entity = nullptr;
    EntityType type = state->type;

    switch (type)
    {
    case ENTITY_EXAMPLE:
        break;
    case ENTITY_DOG:
        entity = std::make_shared<CDogEntity>(id);
        break;
    case ENTITY_HUMAN:
      entity = std::make_shared<CHumanEntity>(id);
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
    case ENTITY_PUDDLE:
      entity = std::make_shared<CPuddleEntity>();
      break;
    case ENTITY_FENCE:
      entity = std::make_shared<CFenceEntity>();
      break;
    case ENTITY_BAR:
      entity = std::make_shared<CBarEntity>();
      break;
    case ENTITY_GATE:
      entity = std::make_shared<CGateEntity>();
      break;
	case ENTITY_TRIGGER:
	  entity = std::make_shared<CTriggerEntity>();
	  break;
	case ENTITY_PLUNGER:
	  entity = std::make_shared<CPlungerEntity>();
	  break;
	case ENTITY_ROPE:
	  entity = std::make_shared<CRopeEntity>();
	  break;
	case ENTITY_TRAP:
	  entity = std::make_shared<CTrapEntity>();
	  break;
	case ENTITY_TREE:
	  entity = std::make_shared<CTreeEntity>();
	  break;
    case ENTITY_FLOOR:
	  // Floor tiles should not be placed in map
      CFloorEntity::getInstance().updateTile(state);
	  return nullptr;
    }

    if (entity)
    {
        _entityList.push_back(entity);
        _entityMap.insert({id, _entityList.size() - 1});
    }

    return entity;
}

void EntityManager::update(std::shared_ptr<BaseState> const &state)
{
	auto entity = getEntity(state);

    if (entity)
    {
        entity->updateState(state);
    }

	ColliderManager::getInstance().updateState(state);

	// Destroy entity if necessary
	if (state->isDestroyed)
    {
        // Find in map and destroy if it exists
        auto result = _entityMap.find(state->id);
        if (result != _entityMap.end())
		{
			//_entityList.erase(_entityList.begin() + result->second);

			// Erase from map
			_entityList[result->second] = nullptr;
			_entityMap.erase(result);
		}

        ColliderManager::getInstance().erase(state->id);

        return;
    }
}

void EntityManager::render(std::unique_ptr<Camera> const &camera)
{
    std::vector<bool> test(_entityList.size());
    std::vector<CBaseEntity *> transparentEntities;
    std::vector<int> isOpaque;

    for (uint32_t i = 0; i < _entityList.size(); i++)
    {
		if (_entityList[i]) {
			const glm::vec3 pos = _entityList[i]->getPos();
			const float radius = _entityList[i]->getRadius();
			test[i] = camera->isInFrustum(pos, radius);
			if (test[i])
			{
				_entityList[i]->setAlpha(camera->getTransparency(pos, radius));
				if (_entityList[i]->getAlpha() < 1.0f || _entityList[i]->getType() == ENTITY_BOX)
				{
					if (_entityList[i]->getAlpha() > 0.01f) {
						transparentEntities.push_back(_entityList[i].get());
					}
				}
				else
				{
					isOpaque.push_back(i);
				}
			}
		}
    }

    // save off current state of src / dst blend functions
    GLint blendSrc;
    GLint blendDst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);

    // Render Opaque object
    for (uint32_t i = 0; i < isOpaque.size(); i++)
    {
        _entityList[isOpaque[i]]->render(camera);
    }

    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render Transparent objects from far to close
    std::sort(transparentEntities.begin(), transparentEntities.end(),
              [&](const CBaseEntity *a, const CBaseEntity *b) {
                  glm::vec3 aPos = a->getPos();
                  glm::vec3 bPos = b->getPos();
                  float aDist = glm::length2(aPos - camera->position());
                  float bDist = glm::length2(bPos - camera->position());
                  return aDist > bDist;
              });

    for (uint32_t i = 0; i < transparentEntities.size(); i++)
    {
        transparentEntities[i]->render(camera);
    }

    // restore
    glEnable(GL_CULL_FACE);
    glBlendFunc(blendSrc, blendDst);

    // render collider
    ColliderManager::getInstance().render(camera);
}

void EntityManager::clearAll() {
	_entityList.clear();
	_entityMap.clear();
}
