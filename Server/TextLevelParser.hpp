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
		UP, // Only used in walls or fences
		DOWN,
		RIGHT // Left is never used
	};

	// Internal representation of tiles
	class Tile
	{
	public:
		TileType type;
		glm::vec2 pos;
		float xIndex;
		float zIndex;
		bool isClaimed;

		void aggregateTiles(TileType type, Direction dir, std::vector<Tile*>& aggregatedTiles);
	};
};

