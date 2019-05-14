#pragma once

#include <queue>
#include "LevelParser.hpp"

/**
  * Parser for tile-based level editor, which exports tiles in a newline-
  * separated file. The level is assumed to have the dimensions of a square.
  */
class GridLevelParser : public LevelParser
{
public:
	GridLevelParser();
	~GridLevelParser();

	std::vector<std::shared_ptr<SBaseEntity>> parseLevelFromFile(
		std::string path,
		std::vector<glm::vec2> & jailLocations,
		std::queue<glm::vec2> & humanSpawns,
		std::queue<glm::vec2> & dogSpawns) override;

private:
	// Different types of tiles; order must be exactly same as online editor!
	enum TileType
	{
		TILE_EMPTY,
		TILE_JAIL,
		TILE_WALL,
		TILE_FENCE,
		TILE_HUMAN_SPAWN,
		TILE_DOG_SPAWN,
		TILE_HOUSE_6X6_A,
		TILE_DOGBONE,
		TILE_DOGHOUSE,
		TILE_HYDRANT,
		TILE_FOUNTAIN
	};

	// Order must be exactly the same as the online editor!
	enum GroundType
	{
		GROUND_GRASS,
		GROUND_ROAD,
		GROUND_DIRT
	};

	// Direction of aggregation
	enum Direction
	{
		DIR_NONE, // Base case in recursion
		DIR_UP, // Only used in walls or fences
		DIR_DOWN,
		DIR_RIGHT // Left is never used
	};

	// Internal representation of tiles
	class Tile
	{
	public:
		TileType type;
		GroundType groundType;
		glm::vec3 forward;
		glm::vec2 pos;
		int xIndex;
		int zIndex;
		bool isClaimed;
		bool isDoublyClaimed;

		void aggregateTiles(
			Tile*** tiles,
			TileType type,
			Direction dir,
			std::vector<Tile*>& aggregatedTiles,
			int maxWidth);
	};

	// Get average position, width, and depth from tiles. All are passed by
	// reference.
	void getTileInfo(
		std::vector<Tile*> & tiles,
		glm::vec2 & avgPos,
		float & width,
		float & depth);
};

