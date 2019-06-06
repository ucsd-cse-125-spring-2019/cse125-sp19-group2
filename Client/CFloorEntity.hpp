#pragma once

#include "CBaseEntity.hpp"
#include "CGrassEntity.hpp"
#include "Model.hpp"
#include "FrameBuffer.h"

#define FLOOR_TEXTURE_SCALE 30

class CFloorEntity : public CBaseEntity
{
private:
	CFloorEntity();
	static std::vector<std::vector<FloorType>> _floorMap;
	static std::vector<std::shared_ptr<CGrassEntity>> _grassList;
	float tileScale;
	unsigned int dirtTextureID;
	unsigned int roadTextureID;
	std::unique_ptr<FrameBuffer> fbo;
	bool updatedTexture;
	std::unique_ptr<Shader> _textureShader;
	bool isGrassIntialized;

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
	* \brief Render floor
	* \param camera(std::unique_ptr<Camera> const&) Local camera
	*/
	void render(std::unique_ptr<Camera> const& camera) override;

	/**
	* \brief Generate floor textures for each type of tiles
	* \param camera(std::unique_ptr<Camera> const&) Local camera
	*/
	void createFloorTexture(std::unique_ptr<Camera> const& camera);

	// Deletes all grass entities
	void reset();

	void initGrass();
};

