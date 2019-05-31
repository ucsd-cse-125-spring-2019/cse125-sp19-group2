#pragma once

#include <queue>
#include "LevelParser.hpp"
#include <map>

/**
  * Parser for tile-based level editor, which exports tiles in a newline-
  * separated file. The level is assumed to have the dimensions of a square.
  */
class GridLevelParser : public LevelParser
{
public:
	GridLevelParser();
	~GridLevelParser();

	void parseLevelFromFile(
		std::string path,
		StructureInfo* structureInfo) override;

private:
	// Different types of tiles; order must be exactly same as online editor!
	enum TileType
	{
		TILE_EMPTY,
		TILE_JAIL,
		TILE_WALL, // should not have this anymore
		TILE_FENCE,
		TILE_HUMAN_SPAWN,
		TILE_DOG_SPAWN,
		TILE_HOUSE_6X6_A,
		TILE_DOGBONE,
		TILE_DOGHOUSE,
		TILE_HYDRANT,
		TILE_FOUNTAIN,
		TILE_TREE
	};

	// Different width for different structure (0 means size is not fixed)
	std::map<TileType, int> EntityTileWidth = {
		{TILE_EMPTY, 1},
		{TILE_JAIL, 0},
		{TILE_WALL, 0},
		{TILE_FENCE, 0},
		{TILE_HUMAN_SPAWN, 1},
		{TILE_DOG_SPAWN, 1},
		{TILE_HOUSE_6X6_A, 4},
		{TILE_DOGBONE, 1},
		{TILE_DOGHOUSE, 1},
		{TILE_HYDRANT, 1},
		{TILE_FOUNTAIN, 3},
		{TILE_TREE, 1}
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
		int angle; // Number of times to rotate clockwise
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
			int maxWidth,
			int maxDepth);
	};

	// Get average position, width, and depth from tiles. All are passed by
	// reference.
	void getTileInfo(
		std::vector<Tile*> & tiles,
		glm::vec2 & avgPos,
		float & width,
		float & depth);
};

