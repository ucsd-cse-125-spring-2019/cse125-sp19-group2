#pragma once
#include "CPlayerEntity.hpp"
#include <GLFW/glfw3.h>
#include "NetworkClient.hpp"

class LocalPlayer {
private:

    uint32_t _playerId;

    std::unique_ptr<Camera> _camera;
    std::shared_ptr<CPlayerEntity> _playerEntity;

	glm::vec3 _offset;
	float _pitch = -45.0f;
	float _distance = 3.0f;

public:
    LocalPlayer(uint32_t playerId, std::unique_ptr<NetworkClient> const& networkClient);

    void update();

    std::unique_ptr<Camera> const& getCamera();
};
