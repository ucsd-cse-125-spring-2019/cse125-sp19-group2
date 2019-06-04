﻿#pragma once
#include "CPlayerEntity.hpp"
#include <GLFW/glfw3.h>
#include "NetworkClient.hpp"
#include "GamePadXbox.hpp"
/**
 * \brief A class that handles the entity corresponding to local player and camera movement.
 */
class LocalPlayer
{
private:
    uint32_t _playerId;

    std::unique_ptr<Camera> _camera;
    std::unique_ptr<GamePadXbox> _gamePad;
    PlayerType _playerType;

    std::shared_ptr<CPlayerEntity> _playerEntity;
    NetworkClient *_networkClient;

    float _height;

    bool _moveKeysPressed, _stopped, _moveCamera;

	// Skill type to use (only for humans)
	bool _usePlunger = true; // False for trap, true for plunger

	// Whether or not the bumpers are pressed for XBOX gamepad
	bool _leftBumperDown = false;
	bool _rightBumperDown = false;

public:
    /**
     * \brief Constructor of local player.
     * \param playerId(uint32_t) Entity id of local player
     * \param networkClient(std::unique_ptr<NetworkClient> const&) networkClient for sending events
     */
    LocalPlayer(uint32_t playerId, std::unique_ptr<NetworkClient> const &networkClient);

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
    std::unique_ptr<Camera> const &getCamera();

	/**
	 * \brief Set the number of the controller
	 * Usage: setControllerNum(GamePadIndex_One)
	 * \param GamePadIndex(int) GamePadIndex Keycode
	 * \return true if success
	 */
	bool setControllerNum(GamePadIndex player);

	// Sets the _playerEntity pointer to null, and allows the LocalPlayer to
	// register a new player entity
	void unpairEntity();

	/**
	 * \brief Set the type of player
	 * Usage: setPlayerType(Player_Dog)
	 * \param PlayerType(int) Player_Dog = 0, Player_Human = 1
	 * \return void
	 */
	void setPlayerType(PlayerType typeNum);

	void setMouseCaptured(bool mouseCaptured);

	bool getMouseCaptured();

	PlayerType getPlayerType();

	uint32_t getPlayerId();

    /**
     * \brief Function needs to be called when the window resizes
     * \param x(int) Width of the window
     * \param y(int) Height of the window
     */
    void resize(int x, int y);

	float getCompassDirection(glm::vec3 pos);

	std::shared_ptr<CPlayerEntity> getPlayerEntity();
};
