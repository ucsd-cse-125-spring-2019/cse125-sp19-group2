#pragma once
#include "CPlayerEntity.hpp"
#include <GLFW/glfw3.h>
#include "NetworkClient.hpp"

/**
 * \brief A class that handles the entity corresponding to local player and camera movement.
 */
class LocalPlayer {
private:

    uint32_t _playerId;

    std::unique_ptr<Camera> _camera;
    std::shared_ptr<CPlayerEntity> _playerEntity;

	glm::vec3 _offset;
	float _pitch = -45.0f;
	float _distance = 14.0f;

public:

    /**
     * \brief Constructor of local player.
     * \param playerId(uint32_t) Entity id of local player
     * \param networkClient(std::unique_ptr<NetworkClient> const&) networkClient for sending events
     */
    LocalPlayer(uint32_t playerId, std::unique_ptr<NetworkClient> const& networkClient);

    /**
     * \brief Update camera based on player entity.
     */
    void update();

    /**
     * \brief Get local camera (for rendering)
     * \return std::unique_ptr<Camera> const&: Local camera
     */
    std::unique_ptr<Camera> const& getCamera();
};
