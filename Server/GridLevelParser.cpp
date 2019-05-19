#include <fstream>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>
#include "GridLevelParser.hpp"
#include "SBoxEntity.hpp"
#include "SHouseEntity.hpp"
#include "SJailEntity.hpp"
#include "SBoneEntity.hpp"
#include "SFloorEntity.hpp"
#include "SDogHouseEntity.hpp"
#include "SHydrantEntity.hpp"
#include "SFountainEntity.hpp"
#include "SFenceEntity.hpp"

GridLevelParser::GridLevelParser()
{
}


GridLevelParser::~GridLevelParser()
{
}


void GridLevelParser::parseLevelFromFile(
	std::string path,
	StructureInfo* structureInfo)
{
	// Allocate structureInfo objects
	structureInfo->entityMap = new std::unordered_map<uint32_t, std::shared_ptr<SBaseEntity>>();
	structureInfo->newEntities = new std::vector<std::shared_ptr<SBaseEntity>>();
	structureInfo->jails = new std::vector<glm::vec2>();
	structureInfo->humanSpawns = new std::queue<glm::vec2>();
	structureInfo->dogSpawns = new std::queue<glm::vec2>();
	structureInfo->dogHouses = new std::vector<std::shared_ptr<SBaseEntity>>();

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

	// Each tile has a type, a ground type, and an orientation
	int tileCount = fileSize / 3;

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
		// Create a tile and put it into our 2D array
		Tile* tile = new Tile();

		// Next byte is ground type
		tile->groundType = (GroundType)levelFile.get();

		// Read next byte as angle
		angle = levelFile.get() * 2; // Angles are encoded as 1/3 their value

		// Cast uint to enum
		tile->type = (TileType)tileType;

		// Set forward based on angle
		tile->angle = angle / 90;

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

			// get maxWidth and maxDepth of this structure
			int structureSize = EntityTileWidth[tile->type];
			int endX = (structureSize) ? structureSize + xIndex : width;
			int endZ = (structureSize) ? structureSize + zIndex : width;

			// Create vector of aggregated tiles and recursively build it
			auto aggregatedTiles = std::vector<Tile*>();
			tile->aggregateTiles(
				tiles,
				tile->type,
				DIR_RIGHT,
				aggregatedTiles,
				endX,
				endZ);

			// Build game entity from aggregated tiles
			if (aggregatedTiles.size())
			{

				// Get general info from tiles first
				glm::vec2 avgPos;
				float entityWidth, entityDepth;
				getTileInfo(aggregatedTiles, avgPos, entityWidth, entityDepth);

				// For fences, one dimension will be 0
				if (aggregatedTiles[0]->type == TILE_FENCE)
				{
					entityWidth += FENCE_WIDTH;
					entityDepth += FENCE_WIDTH;

					// North <--> South fence
					if (entityDepth > entityWidth)
					{
						// Tiles are already sorted by Z. Retract
						// fence1 as necessary
						if (aggregatedTiles[0]->isDoublyClaimed)
						{
							// North end
							entityDepth -= FENCE_WIDTH;
							avgPos.y += FENCE_WIDTH / 2;
						}
						if (aggregatedTiles.back()->isDoublyClaimed)
						{
							// South end
							entityDepth -= FENCE_WIDTH;
							avgPos.y -= FENCE_WIDTH / 2;
						}

						// TODO: extend fence to connect to neighboring buildings
					}

					// West <--> East fence
					else if (entityWidth > entityDepth)
					{
						// Sort tiles by X
						std::sort(aggregatedTiles.begin(), aggregatedTiles.end(),
							[](const Tile* a, const Tile* b) -> bool
						{
							return a->pos.x < b->pos.x;
						});

						// Retract fence as necessary
						if (aggregatedTiles[0]->isDoublyClaimed)
						{
							// West end
							entityWidth -= FENCE_WIDTH;
							avgPos.x += FENCE_WIDTH/2;
						}
						if (aggregatedTiles.back()->isDoublyClaimed)
						{
							// East end
							entityWidth -= FENCE_WIDTH;
							avgPos.x -= FENCE_WIDTH/2;
						}

						// TODO: extend fences to connect to neighboring buildings
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
						entity = std::make_shared<SFenceEntity>(
							glm::vec3(avgPos.x, 0, avgPos.y),
							glm::vec3(entityWidth, FENCE_HEIGHT, entityDepth));
						break;
					}
					case TILE_JAIL:
					{
						entity = std::make_shared<SJailEntity>(
							glm::vec3(avgPos.x, 0, avgPos.y),
							glm::vec3(entityWidth, 2, entityDepth));
						structureInfo->jails->push_back(avgPos);
						break;
					}
					case TILE_HUMAN_SPAWN:
					{
						structureInfo->humanSpawns->push(avgPos);
						break;
					}
					case TILE_DOG_SPAWN:
					{
						structureInfo->dogSpawns->push(avgPos);
						break;
					}
					case TILE_HOUSE_6X6_A:
					{
						// 6x6 red house
						entity = std::make_shared<SHouseEntity>(
							ENTITY_HOUSE_6X6_A,
							glm::vec3(avgPos.x, 0, avgPos.y),
							glm::vec3(6));
						break;
					}
					case TILE_DOGBONE:
					{
						entity = std::make_shared<SBoneEntity>(
							glm::vec3(avgPos.x, 0, avgPos.y));
						break;
					}
					case TILE_DOGHOUSE:
					{
						entity = std::make_shared<SDogHouseEntity>(
							glm::vec3(avgPos.x, 0, avgPos.y),
							structureInfo->dogHouses);
						structureInfo->dogHouses->push_back(entity);
						break;
					}
					case TILE_HYDRANT:
					{
						entity = std::make_shared<SHydrantEntity>(
							glm::vec3(avgPos.x, 0, avgPos.y));
						break;
					}
					case TILE_FOUNTAIN:
					{
						entity = std::make_shared<SFountainEntity>(
							glm::vec3(avgPos.x, 0, avgPos.y));
						break;
					}
				}

				if (entity)
				{
					// Rotate first
					if (aggregatedTiles[0]->angle != 0)
					{
						entity->rotate(entity->getState()->pos, aggregatedTiles[0]->angle);

					}

					// Add entity to map
					structureInfo->entityMap->insert({ entity->getState()->id, entity });

					// Add children to map
					auto children = entity->getChildren();
					for (auto& child : entity->getChildren())
					{
						structureInfo->entityMap->insert({ child->getState()->id, child });
					}
				}
			}

			// Build floor tile if not default floor
			if (tile->groundType != 0)
			{
				auto floorTile = std::make_shared<SFloorEntity>(
					(FloorType)tile->groundType,
					xIndex,
					zIndex,
					tileWidth);
				structureInfo->entityMap->insert({ floorTile->getState()->id, floorTile });
			}
		}
	}
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
	int maxWidth,
	int maxDepth)
{
	if (this->type != type || this->type == TILE_EMPTY ||
		(this->isClaimed && aggregatedTiles.size() == 0))
	{
		return;
	}
	else
	{
		// Edge case for fences; reclaimed tile while moving to right
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
				// Edge case for fences: aggregate single item to left
				if (this->type == TILE_FENCE &&
					xIndex - 1 >= 0 &&
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
						maxWidth,
						maxDepth);
				}

				// If this is a fence and the vector has more than one
				// element, we're done. We don't want fences in multiple
				// dimensions.
				if ((this->type == TILE_FENCE) &&
					aggregatedTiles.size() > 1)
				{
					return;
				}

				// Edge case for fences: aggregate single item above
				if (this->type == TILE_FENCE &&
					zIndex - 1 >= 0 &&
					tiles[zIndex - 1][xIndex]->type == this->type &&
					aggregatedTiles.size() == 1)
				{
					tiles[zIndex - 1][xIndex]->isDoublyClaimed = true;
					aggregatedTiles.push_back(tiles[zIndex - 1][xIndex]);
				}

				// Also check downwards (don't add break here)
			}
			case DIR_DOWN:
			{
				if (this->zIndex + 1 < maxDepth)
				{
					tiles[zIndex + 1][xIndex]->aggregateTiles(
						tiles,
						type,
						DIR_DOWN,
						aggregatedTiles,
						maxWidth,
						maxDepth);
				}
				break;
			}
			// Other cases are unlikely and may not be needed
		}
	}
}
