#pragma once
#include "CPlayerEntity.hpp"
#include <GLFW/glfw3.h>
#include "NetworkClient.hpp"
#include "GamePadXbox.hpp"

/**
 * \brief A class that handles the entity corresponding to local player and camera movement.
 */
class LocalPlayer {
private:

    uint32_t _playerId;

    std::unique_ptr<Camera> _camera;
    std::unique_ptr<GamePadXbox> _pad;
    std::shared_ptr<CPlayerEntity> _playerEntity;
	NetworkClient* _networkClient;


	glm::vec3 _offset;
	float _pitch = -45.0f;
	float _distance = 14.0f;

	bool _moveKeysPressed, _stopped;
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
    
	/*
	** Runs in its own thread; handle input from controller
	*/
	void updateController();

    /**
     * \brief Get local camera (for rendering)
     * \return std::unique_ptr<Camera> const&: Local camera
     */
    std::unique_ptr<Camera> const& getCamera();

    /**
     * \brief Function needs to be called when the window resizes
     * \param x(int) Width of the window
     * \param y(int) Height of the window
     */
    void resize(int x, int y);
};
