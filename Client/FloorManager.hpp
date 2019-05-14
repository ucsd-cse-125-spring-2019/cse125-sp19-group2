#pragma once
#include "CBaseEntity.hpp"
#include "Camera.hpp"
#include "Model.hpp"

class FloorManager
{
private:
	FloorManager();
	static std::vector<std::vector<FloorType>> _floorMap;
	static std::unique_ptr<Model> _floorModel;
	static std::unique_ptr<Shader> _shader;

public:
	/**
	* \brief The singleton getter of FloorManager (create one if not exist)
	* \return FloorManager&: An FloorManager Object
	*/
	static FloorManager& getInstance();

	/**
	* \brief Add new floor 
	* \param state(std::shared_ptr<BaseState> const&) Update state
	*/
	void update(std::shared_ptr<BaseState> const& state);

	/**
	* \brief Render all collision boxes in the game world
	* \param camera(std::unique_ptr<Camera> const&) Local camera
	*/
	void render(std::unique_ptr<Camera> const& camera);
};