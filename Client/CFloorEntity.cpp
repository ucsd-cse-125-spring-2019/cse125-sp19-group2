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
	_state->pos = glm::vec3(0.0f, -0.01f, 0.0f);
	_state->forward = glm::vec3(0, 0, 1.0f);
	_state->up = glm::vec3(0, 1.0f, 0);

	_state->transparency = 1.0f;

	dirtTextureID = LoadTextureFromFile("brown.jpg", "./Resources/Textures");
	roadTextureID = LoadTextureFromFile("road.jpg", "./Resources/Textures");
	fbo = std::make_unique<FrameBuffer>(MAP_WIDTH * FLOOR_TEXTURE_SCALE, MAP_WIDTH * FLOOR_TEXTURE_SCALE);

	_textureShader = std::make_unique<Shader>();
	_textureShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/floorTexture.vert");
	_textureShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/floorTexture.frag");
	_textureShader->CreateProgram();


	updatedTexture = false;
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

	updatedTexture = false;
}

void CFloorEntity::render(std::unique_ptr<Camera> const & camera)
{	
	// create new texture for the floor if not created yet or outdated
	//if (!updatedTexture) {
	//	createFloorTexture(camera);
	//}

	// render the floor with changed texture
	_objectShader->Use();
	setUniforms(camera);

	Mesh floorMesh = (static_cast<Model*>(_objectModel.get()))->getMeshAt(0);

	floorMesh.Draw(_objectShader);

	const auto s = glm::scale(glm::mat4(1.0f), glm::vec3(tileScale, 1, tileScale));

	for (int x = 0; x < _floorMap.size(); x++) {
		for (int z = 0; z < _floorMap[0].size(); z++) {
			// skip the tile that is default
			if (_floorMap[x][z] == FLOOR_GRASS) continue;

			// get actual position and scale of tile
			float xPos = ((float)x * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
			float zPos = ((float)z * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
			auto t = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, 0.002f, zPos));
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

	//floorMesh.Draw(_objectShader, fbo->getRGBA());

}

void CFloorEntity::createFloorTexture(std::unique_ptr<Camera> const & camera)
{
	// save previous framebuffer and viewport
	GLint old_fbo;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_fbo);
	GLint old_viewport[4];
	glGetIntegerv(GL_VIEWPORT, old_viewport);

	_textureShader->Use();
	fbo->renderScene([&]
	{
		Logger::getInstance()->debug("Texture generating");
		glViewport(0, 0, MAP_WIDTH * FLOOR_TEXTURE_SCALE, MAP_WIDTH * FLOOR_TEXTURE_SCALE);
		//Mesh floorMesh = (static_cast<Model*>(_objectModel.get()))->getMeshAt(0);
		auto s = glm::scale(glm::mat4(1.0f), glm::vec3(MAP_WIDTH, 1, MAP_WIDTH));
		_textureShader->set_uniform("u_model", s);

		float halfMapWidth = MAP_WIDTH / 2;

		auto projM = glm::ortho(-halfMapWidth, halfMapWidth, halfMapWidth, -halfMapWidth, 0.1f, (float)MAP_WIDTH);
		auto viewM = glm::lookAt(glm::vec3(0, halfMapWidth, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

		_textureShader->set_uniform("u_projection", projM);
		_textureShader->set_uniform("u_view", viewM);

		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		Mesh floorMesh = (static_cast<Model*>(_objectModel.get()))->getMeshAt(0);
		floorMesh.Draw(_textureShader);

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
				_textureShader->set_uniform("u_model", model);

				// render the tile according to the floor type
				switch (_floorMap[x][z])
				{
				case FLOOR_DIRT:
					floorMesh.Draw(_textureShader, dirtTextureID);
					break;
				case FLOOR_ROAD:
					floorMesh.Draw(_textureShader, roadTextureID);
					break;
				}
			}
		}

		glEnable(GL_DEPTH_TEST);
	});
	updatedTexture = true;

	// bind the previous framebuffer and viewport after drawing the texture
	glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
	glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
}