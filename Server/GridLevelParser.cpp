#include <fstream>
#include <algorithm>
#include "GridLevelParser.hpp"
#include "SBoxEntity.hpp"
#include "SHouseEntity.hpp"
#include "SJailEntity.hpp"

GridLevelParser::GridLevelParser()
{
}


GridLevelParser::~GridLevelParser()
{
}


std::vector<std::shared_ptr<SBaseEntity>> GridLevelParser::parseLevelFromFile(
	std::string path,
	std::vector<glm::vec2> & jailLocations,
	std::queue<glm::vec2> & humanSpawns,
	std::queue<glm::vec2> & dogSpawns)
{
	// List containing game objects parsed from level file
	auto entityList = std::vector<std::shared_ptr<SBaseEntity>>();

	// Open level file
	std::ifstream levelFile;
	try
	{
		levelFile = std::ifstream(path, std::ios_base::binary);
	}
	catch (std::runtime_error e)
	{
		Logger::getInstance()->fatal("Level file does not exist: " + path);
		fgetc(stdin);
		exit(1);
	}

	// Get size of file
	levelFile.seekg(0, std::ios::end);
	int fileSize = (int)levelFile.tellg();
	levelFile.seekg(0, std::ios::beg);

	// Each tile has a type and an orientation
	int tileCount = fileSize / 2;

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

	uint8_t tileType = levelFile.get();
	int angle; // Angle of tile in degrees

	// Read until end of file
	while (zIndex < width)
	{
		// Read next byte as angle
		angle = levelFile.get() * 2; // Angles are encoded as 1/3 their value

		// Create a tile and put it into our 2D array
		Tile* tile = new Tile();

		// Cast uint to enum
		tile->type = (TileType)tileType;

		// Set forward based on angle
		switch (angle)
		{
		case 0:
			tile->forward = glm::vec3(0, 0, -1);
			break;
		case 90:
			tile->forward = glm::vec3(-1, 0, 0);
			break;
		case 180:
			tile->forward = glm::vec3(0, 0, 1);
			break;
		case 270:
			tile->forward = glm::vec3(1, 0, 0);
			break;
		}

		// Other properties
		tile->isClaimed = false;
		tile->isDoublyClaimed = false;
		tile->xIndex = xIndex;
		tile->zIndex = zIndex;

		// Scale positions based on map size and tile count
		float xPos = ((float)xIndex * tileWidth) - (MAP_WIDTH / 2) + tileWidth / 2;
		float zPos = ((float)zIndex * tileWidth) - (MAP_WIDTH / 2) + tileWidth / 2;
		tile->pos = glm::vec2(xPos, zPos);

		// Toss it into the array
		tiles[zIndex][xIndex] = tile;

		// Increment indices appropriately
		xIndex = (xIndex + 1) % width;
		if (xIndex == 0)
		{
			zIndex += 1;
		}

		tileType = levelFile.get();
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

					// North <--> South wall
					if (entityDepth > entityWidth)
					{
						// Tiles are already sorted by Z. Retract
						// wall as necessary
						if (aggregatedTiles[0]->isDoublyClaimed)
						{
							// North end
							entityDepth -= WALL_WIDTH;
							avgPos.y += WALL_WIDTH / 2;
						}
						if (aggregatedTiles.back()->isDoublyClaimed)
						{
							// South end
							entityDepth -= WALL_WIDTH;
							avgPos.y -= WALL_WIDTH / 2;
						}

						// TODO: extend walls to connect to neighboring buildings
					}

					// West <--> East wall
					else if (entityWidth > entityDepth)
					{
						// Sort tiles by X
						std::sort(aggregatedTiles.begin(), aggregatedTiles.end(),
							[](const Tile* a, const Tile* b) -> bool
						{
							return a->pos.x < b->pos.x;
						});

						// Retract wall as necessary
						if (aggregatedTiles[0]->isDoublyClaimed)
						{
							// West end
							entityWidth -= WALL_WIDTH;
							avgPos.x += WALL_WIDTH/2;
						}
						if (aggregatedTiles.back()->isDoublyClaimed)
						{
							// East end
							entityWidth -= WALL_WIDTH;
							avgPos.x -= WALL_WIDTH/2;
						}

						// TODO: extend walls to connect to neighboring buildings
					}
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
							glm::vec3(avgPos.x, 0, avgPos.y),
							aggregatedTiles[0]->forward,
							glm::vec3(entityWidth, WALL_HEIGHT, entityDepth));
						break;
					}
					case TILE_WALL:
					{
						// Same as above; will be changed eventually
						entity = std::make_shared<SBoxEntity>(
							glm::vec3(avgPos.x, 0, avgPos.y),
							aggregatedTiles[0]->forward,
							glm::vec3(entityWidth, WALL_HEIGHT, entityDepth));
						break;
					}
					case TILE_JAIL:
					{
						entity = std::make_shared<SJailEntity>(
							glm::vec3(avgPos.x, 0, avgPos.y),
							aggregatedTiles[0]->forward,
							glm::vec3(entityWidth, 2, entityDepth));
						jailLocations.push_back(avgPos);
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
					case TILE_HOUSE_6X6_A:
					{
						// 6x6 red house
						entity = std::make_shared<SHouseEntity>(
							ENTITY_HOUSE_6X6_A,
							glm::vec3(avgPos.x, 0, avgPos.y),
							aggregatedTiles[0]->forward,
							glm::vec3(6,6,6));
						break;
					}
					case TILE_DOGBONE:
					{
						// Dog bone to refill dog stamina
						// TODO

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

void GridLevelParser::getTileInfo(
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

void GridLevelParser::Tile::aggregateTiles(
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
		// Edge case for walls; reclaimed tile while moving to right
		if (this->isClaimed)
		{
			this->isDoublyClaimed = true;
		}

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
					tiles[zIndex][xIndex - 1]->isDoublyClaimed = true;
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
					tiles[zIndex - 1][xIndex]->isDoublyClaimed = true;
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
