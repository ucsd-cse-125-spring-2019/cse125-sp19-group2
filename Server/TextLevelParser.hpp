#pragma once

#include "LevelParser.hpp"

/**
  * Parser for tile-based level editor, which exports tiles in a newline-
  * separated file. The level is assumed to have the dimensions of a square.
  */
class TextLevelParser : public LevelParser
{
public:
	TextLevelParser();
	~TextLevelParser();

	std::vector<std::shared_ptr<SBaseEntity>> parseLevelFromFile(std::string path) override;

private:
	// Different types of tiles
	enum TileType
	{
		TILE_EMPTY,
		TILE_JAIL,
		TILE_WALL,
		TILE_FENCE
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
		glm::vec2 pos;
		int xIndex;
		int zIndex;
		bool isClaimed;

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

