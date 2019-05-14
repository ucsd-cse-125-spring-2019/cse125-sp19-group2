#pragma once
#include "CBaseEntity.hpp"

struct FloorTile
{
	FloorType type;

};

class FloorManager
{
private:
	FloorManager();


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
};