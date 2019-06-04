#pragma once
#include <unordered_map>
#include "CBaseEntity.hpp"

/**
 * \brief Manage all entities. Handle entities' updating and rendering.
 */
class EntityManager {
private:
    EntityManager();

	std::unordered_map<uint32_t, int> _entityMap;
    std::vector<std::shared_ptr<CBaseEntity>> _entityList;
public:
	/**
	 * \brief The singleton getter of EntityManager (create one if not exist)
	 * \return EntityManager&: An EntityManager Object
	 */
    static EntityManager& getInstance();

    /**
     * \brief Get an existing entity using entity id, return nullptr if not exist
     * \param id(uint32_t) Entity Id
     * \return std::shared_ptr<CBaseEntity>: Share pointer to the Entity
     */
    std::shared_ptr<CBaseEntity> getEntity(uint32_t id);

    /**
     * \brief Get an existing entity using BaseState, create one if not exist
     * \param state(std::shared_ptr<BaseState> const&) Entity state
     * \return std::shared_ptr<CBaseEntity>: Share pointer to the Entity
     */
    std::shared_ptr<CBaseEntity> getEntity(std::shared_ptr<BaseState> const& state);

	std::vector<uint32_t> getEntityIdList();

	int getEntityCount();

    /**
     * \brief Dispatch new state to the entity to update entity
     * \param state(std::shared_ptr<BaseState> const&) Update state
     */
    void update(std::shared_ptr<BaseState> const& state);

    /**
     * \brief Render all entities in the game world
     * \param camera(std::unique_ptr<Camera> const&) Local camera
     */
    void render(std::unique_ptr<Camera> const& camera);

	/**
	 * \brief Deletes all entities from the server
	 */
	void clearAll();
};
