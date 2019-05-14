#include "CFloorEntity.hpp"
#include "Shared/Logger.hpp"
#include <string.h>

std::vector<std::vector<FloorType>> CFloorEntity::_floorMap;

CFloorEntity::CFloorEntity()
{
	_objectModel = std::make_unique<Model>("./Resources/Models/floor_tile.fbx");
	_objectShader = std::make_unique<Shader>();

	_objectShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
	_objectShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/basiclight.frag");
	_objectShader->CreateProgram();

	_state = std::make_shared<BaseState>();

	_state->scale = glm::vec3(MAP_WIDTH, 1, MAP_WIDTH);
	_state->pos = glm::vec3(0);
	_state->forward = glm::vec3(0, 0, 1.0f);
	_state->up = glm::vec3(0, 1.0f, 0);

	dirtTextureID = LoadTextureFromFile("brown.jpg", "./Resources/Textures");
	roadTextureID = LoadTextureFromFile("road.jpg", "./Resources/Textures");
}

CFloorEntity & CFloorEntity::getInstance()
{
	static CFloorEntity floorEntity;
	return floorEntity;
}

CFloorEntity::~CFloorEntity()
{
}

void CFloorEntity::updateTile(std::shared_ptr<BaseState> const & state)
{
	// initialize the 2D array if not initialized
	if (_floorMap.empty()) {
		tileScale = state->width;
		int numTiles = MAP_WIDTH / tileScale;
		std::vector<FloorType> temp(numTiles, FLOOR_GRASS);
		_floorMap = std::vector<std::vector<FloorType>>(numTiles, temp);
	}

	_floorMap[(int)(state->pos.x)][(int)(state->pos.z)] = (FloorType)((int)(state->height));
}

void CFloorEntity::render(std::unique_ptr<Camera> const & camera)
{
	// render the default floor
	_objectShader->Use();
	setUniforms(camera);
	glDisable(GL_DEPTH_TEST);
	_objectModel->render(_objectShader);
	glEnable(GL_DEPTH_TEST);

	// render all floor tiles that are not default
	Mesh floorMesh = (static_cast<Model*>(_objectModel.get()))->getMeshAt(0);
	for (int x = 0; x < _floorMap.size(); x++) {
		for (int z = 0; z < _floorMap[0].size(); z++) {
			// skip the tile that is default
			if (_floorMap[x][z] == FLOOR_GRASS) continue;

			// get actual position and scale of tile
			float xPos = ((float)x * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
			float zPos = ((float)z * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
			auto t = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, 0, zPos));
			auto s = glm::scale(glm::mat4(1.0f), glm::vec3(tileScale, 1, tileScale));
			auto model = t * s;
			_objectShader->set_uniform("u_model", model);

			// render the tile according to the floor type
			switch (_floorMap[x][z])
			{
			case FLOOR_DIRT:
				floorMesh.Draw(_objectShader, dirtTextureID);
				break;
			case FLOOR_ROAD:
				floorMesh.Draw(_objectShader, roadTextureID);
				break;
			}
		}
	}

}
