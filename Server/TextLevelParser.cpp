#include <fstream>
#include <algorithm>
#include "TextLevelParser.hpp"
#include "SBoxEntity.hpp"
#include "SJailEntity.hpp"

TextLevelParser::TextLevelParser()
{
}


TextLevelParser::~TextLevelParser()
{
}


std::vector<std::shared_ptr<SBaseEntity>> TextLevelParser::parseLevelFromFile(
	std::string path,
	std::queue<glm::vec2> & jailLocations,
	std::queue<glm::vec2> & humanSpawns,
	std::queue<glm::vec2> & dogSpawns)
{
	// List containing game objects parsed from level file
	auto entityList = std::vector<std::shared_ptr<SBaseEntity>>();

	// 2 passes over file: first pass determines dimensions of map
	std::ifstream levelFile;
	try
	{
		levelFile = std::ifstream(path);
	}
	catch (std::runtime_error e)
	{
		Logger::getInstance()->fatal("Level file does not exist: " + path);
		fgetc(stdin);
		exit(1);
	}

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
		fgetc(stdin);
		exit(1);
	}

	int width = (int)sr;
	float tileWidth = MAP_WIDTH / (float)width;

	// Create 2D array of tiles
	Tile*** tiles = new Tile**[width];
	for (int i = 0; i < width; i++)
	{
		tiles[i] = new Tile*[width];
	}

	int xIndex = 0;
	int zIndex = 0;

	std::string tileType;
	int angle; // Angle of tile in degrees

	// Re-init file and loop over tiles
	levelFile = std::ifstream(path);
	while (levelFile >> tileString)
	{
		tileType = tileString.substr(0, tileString.find(","));
		tileString.erase(0, tileType.size() + 1);
		angle = std::stoi(tileString);

		// Create a tile and put it into our 2D array
		Tile* tile = new Tile();

		// Assign type based on string
		TileType type = TILE_EMPTY;
		if (tileType == "wall-tile")
		{
			type = TILE_WALL;
		}
		else if (tileType == "fence-tile")
		{
			type = TILE_FENCE;
		}
		else if (tileType == "jail-tile")
		{
			type = TILE_JAIL;
		}
		else if (tileType == "human-spawn-tile")
		{
			type = TILE_HUMAN_SPAWN;
		}
		else if (tileType == "dog-spawn-tile")
		{
			type = TILE_DOG_SPAWN;
		}
		tile->type = type;

		// Set forward based on angle
		switch (angle)
		{
		case 0:
			tile->forward = glm::vec3(0, 0, -1);
			break;
		case 90:
			tile->forward = glm::vec3(1, 0, 0);
			break;
		case 180:
			tile->forward = glm::vec3(0, 0, 1);
			break;
		case 270:
			tile->forward = glm::vec3(-1, 0, 0);
			break;
		}

		// Other properties
		tile->isClaimed = false;
		tile->xIndex = xIndex;
		tile->zIndex = zIndex;
		
		// Scale positions based on map size and tile count
		float xPos = ((float)xIndex * tileWidth) - (MAP_WIDTH / 2) + tileWidth/2;
		float zPos = ((float)zIndex * tileWidth) - (MAP_WIDTH / 2) + tileWidth/2;
		tile->pos = glm::vec2(xPos, zPos);
		
		// Toss it into the array
		tiles[zIndex][xIndex] = tile; 

		// Increment indices appropriately
		xIndex = (xIndex + 1) % width;
		if (xIndex == 0)
		{
			zIndex += 1;
		}
	}

	// Iterate over tiles and aggregate them into game entities
	for (zIndex = 0; zIndex < width; zIndex++)
	{
		for (xIndex = 0; xIndex < width; xIndex++)
		{
			Tile* tile = tiles[zIndex][xIndex];

			// Create vector of aggregated tiles and recursively build it
			auto aggregatedTiles = std::vector<Tile*>();
			tile->aggregateTiles(
				tiles,
				tile->type,
				DIR_RIGHT,
				aggregatedTiles,
				width);

			// Build game entity from aggregated tiles
			if (aggregatedTiles.size())
			{

				// Get general info from tiles first
				glm::vec2 avgPos;
				float entityWidth, entityDepth;
				getTileInfo(aggregatedTiles, avgPos, entityWidth, entityDepth);

				// For fences and walls, one dimension will be 0
				if (aggregatedTiles[0]->type == TILE_FENCE ||
					aggregatedTiles[0]->type == TILE_WALL)
				{
					entityWidth += WALL_WIDTH;
					entityDepth += WALL_WIDTH;
				}
				else // For everything else, fit the object tightly to its tiles
				{
					entityWidth += tileWidth;
					entityDepth += tileWidth;
				}
				
				// Entity to build and add to vector
				std::shared_ptr<SBaseEntity> entity = nullptr;

				// Entity-specific handling
				switch (aggregatedTiles[0]->type)
				{
					case TILE_FENCE:
					{
						// For now just create a box entity
						entity = std::make_shared<SBoxEntity>(
							glm::vec3(avgPos.x, WALL_HEIGHT / 2, avgPos.y),
							aggregatedTiles[0]->forward,
							glm::vec3(entityWidth, WALL_HEIGHT, entityDepth));
						break;
					}
					case TILE_WALL:
					{
						// Same as above; will be changed eventually
						entity = std::make_shared<SBoxEntity>(
							glm::vec3(avgPos.x, WALL_HEIGHT/2, avgPos.y),
							aggregatedTiles[0]->forward,
							glm::vec3(entityWidth, WALL_HEIGHT, entityDepth));
						break;
					}
					case TILE_JAIL:
					{
						entity = std::make_shared<SJailEntity>(
							glm::vec3(avgPos.x, 0.15f, avgPos.y),
							aggregatedTiles[0]->forward,
							glm::vec3(entityWidth, 0.3f, entityDepth));
						jailLocations.push(avgPos);
						break;
					}
					case TILE_HUMAN_SPAWN:
					{
						humanSpawns.push(avgPos);
						break;
					}
					case TILE_DOG_SPAWN:
					{
						dogSpawns.push(avgPos);
						break;
					}
				}

				if (entity)
				{
					entityList.push_back(entity);
					auto children = entity->getChildren();
					entityList.insert(entityList.begin(), children.begin(), children.end());
				}
			}
		}
	}

	return entityList;
}

void TextLevelParser::getTileInfo(
	std::vector<Tile*> & tiles,
	glm::vec2 & avgPos,
	float & width,
	float & depth)
{
	// Get average position first
	float totalX = 0;
	float totalZ = 0;
	for (auto& tile : tiles)
	{
		totalX += tile->pos.x;
		totalZ += tile->pos.y;
	}
	avgPos = glm::vec2(totalX / tiles.size(), totalZ / tiles.size());

	// For width, sort vector by X and compare first and last
	std::sort(tiles.begin(), tiles.end(),
		[](const Tile* a, const Tile* b) -> bool
	{
		return a->pos.x < b->pos.x;
	});
	width = std::abs(tiles[0]->pos.x - tiles.back()->pos.x);

	// For depth, sort by Z and compare first and last
	std::sort(tiles.begin(), tiles.end(),
		[](const Tile* a, const Tile* b) -> bool
	{
		return a->pos.y < b->pos.y;
	});
	depth = std::abs(tiles[0]->pos.y - tiles.back()->pos.y);
}

void TextLevelParser::Tile::aggregateTiles(
	Tile*** tiles,
	TileType type,
	Direction dir,
	std::vector<Tile*>& aggregatedTiles,
	int maxWidth)
{
	if (this->type != type || this->type == TILE_EMPTY ||
		(this->isClaimed && aggregatedTiles.size() == 0))
	{
		return;
	}
	else
	{
		this->isClaimed = true;
		aggregatedTiles.push_back(this);
		switch (dir)
		{
			case DIR_RIGHT:
			{
				// Edge case for walls and fences: aggregate single item to left
				if (xIndex - 1 >= 0 &&
					tiles[zIndex][xIndex - 1]->type == this->type &&
					aggregatedTiles.size() == 1)
				{
					aggregatedTiles.push_back(tiles[zIndex][xIndex - 1]);
				}

				// Iterate to the right
				if (this->xIndex + 1 < maxWidth)
				{
					tiles[zIndex][xIndex + 1]->aggregateTiles(
						tiles,
						type,
						DIR_RIGHT,
						aggregatedTiles,
						maxWidth);
				}

				// If this is a wall or fence and the vector has more than one
				// element, we're done. We don't want fences in multiple
				// dimensions.
				if ((this->type == TILE_FENCE || this->type == TILE_WALL) &&
					aggregatedTiles.size() > 1)
				{
					return;
				}

				// Edge case for walls and fences: aggregate single item above
				if (zIndex - 1 >= 0 &&
					tiles[zIndex - 1][xIndex]->type == this->type &&
					aggregatedTiles.size() == 1)
				{
					aggregatedTiles.push_back(tiles[zIndex - 1][xIndex]);
				}

				// Also check downwards. It's a little bit of copied code so
				// I might come back later to try and clean it up
				if (this->zIndex + 1 < maxWidth)
				{
					tiles[zIndex + 1][xIndex]->aggregateTiles(
						tiles,
						type,
						DIR_DOWN,
						aggregatedTiles,
						maxWidth);
				}
				break;
			}
			case DIR_DOWN:
			{
				if (this->zIndex + 1 < maxWidth)
				{
					tiles[zIndex + 1][xIndex]->aggregateTiles(
						tiles,
						type,
						DIR_DOWN,
						aggregatedTiles,
						maxWidth);
				}
				break;
			}
			// Other cases are unlikely and may not be needed
		}
	}
}
