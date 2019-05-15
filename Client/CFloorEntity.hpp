#pragma once

#include "CBaseEntity.hpp"
#include "Model.hpp"
#include "FrameBuffer.h"

#define FLOOR_TEXTURE_SCALE 10

class CFloorEntity : public CBaseEntity
{
private:
	CFloorEntity();
	static std::vector<std::vector<FloorType>> _floorMap;
	float tileScale;
	unsigned int dirtTextureID;
	unsigned int roadTextureID;
	std::unique_ptr<FrameBuffer> fbo;

public:
	/**
	* \brief The singleton getter of FloorManager (create one if not exist)
	* \return FloorManager&: An FloorManager Object
	*/
	static CFloorEntity& getInstance();

	~CFloorEntity();

	/**
	* \brief Add new floor
	* \param state(std::shared_ptr<BaseState> const&) Update state
	*/
	void updateTile(std::shared_ptr<BaseState> const& state);

	/**
	* \brief Render all collision boxes in the game world
	* \param camera(std::unique_ptr<Camera> const&) Local camera
	*/
	void render(std::unique_ptr<Camera> const& camera) override;
};

