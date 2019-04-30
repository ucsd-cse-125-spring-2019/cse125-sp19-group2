#include <fstream>
#include "TextLevelParser.hpp"

TextLevelParser::TextLevelParser()
{
}


TextLevelParser::~TextLevelParser()
{
}


std::vector<std::shared_ptr<SBaseEntity>> TextLevelParser::parseLevelFromFile(std::string path)
{
	// List containing game objects parsed from level file
	auto entityList = std::vector<std::shared_ptr<SBaseEntity>>();

	// 2 passes over file: first pass determines dimensions of map
	std::ifstream levelFile(path);

	std::string tileString;
	int tileCount = 0;
	while (levelFile >> tileString)
	{
		tileCount++;
	}

	// Ensure tile count is a perfect square
	auto sr = std::sqrt(tileCount);
	if ((sr - std::floor(sr)) != 0)
	{
		Logger::getInstance()->fatal("Level file does not have a square map!");
		exit(1);
	}

	int width = (int)sr;

	// Create array of tiles
	Tile*** tiles = new Tile**[width];
	for (int i = 0; i < width; i++)
	{
		tiles[i] = new Tile*[width];
	}

	int xIndex = 0;
	int zIndex = 0;

	// Re-init file and loop over tiles
	levelFile = std::ifstream(path);
	while (levelFile >> tileString)
	{
		// Create a tile and put it into our 2D array
		Tile* tile = new Tile();
		

		// Increment indices appropriately
		xIndex = (xIndex + 1) % width;
		if (xIndex == 0)
		{
			zIndex += 1;
		}
	}

	return entityList;
}

void TextLevelParser::Tile::aggregateTiles(TileType type, Direction dir, std::vector<Tile*>& aggregatedTiles)
{
}
