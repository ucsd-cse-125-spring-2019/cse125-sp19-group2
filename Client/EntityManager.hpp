#pragma once
#include <unordered_map>
#include "CBaseEntity.hpp"
#include <algorithm>

class EntityManager {
private:
    EntityManager();
public:
    static EntityManager& getInstance();

    std::unordered_map<uint32_t, std::shared_ptr<CBaseEntity>> entityList;

    std::shared_ptr<CBaseEntity> getEntity(uint32_t id);
    std::shared_ptr<CBaseEntity> getEntity(std::shared_ptr<BaseState> const& state);

    void update(std::shared_ptr<BaseState> const& state);

    void render(std::unique_ptr<Camera> const& camera) {
        std::for_each(entityList.begin(), entityList.end(), [&camera](std::pair<uint32_t, std::shared_ptr<CBaseEntity>> entity)
        {
            entity.second->render(camera);
        });
    }
};
