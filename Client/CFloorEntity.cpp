#include "CFloorEntity.hpp"
#include "Shared/Logger.hpp"
#include <string.h>
#include "EntityManager.hpp"

std::vector<std::vector<FloorType>> CFloorEntity::_floorMap;
std::vector<glm::mat4> CFloorEntity::_grassList;
std::vector<glm::mat4> CFloorEntity::_pebbleList;
std::vector<glm::mat4> CFloorEntity::_dirtPebbleList;
std::vector<std::vector<bool>> CFloorEntity::_claimedMap;

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

	dirtTextureID = LoadTextureFromFile("dirt.jpg", "./Resources/Textures");
	roadTextureID = LoadTextureFromFile("road.jpg", "./Resources/Textures");
	fbo = std::make_unique<FrameBuffer>(MAP_WIDTH * FLOOR_TEXTURE_SCALE, MAP_WIDTH * FLOOR_TEXTURE_SCALE);

	_textureShader = std::make_unique<Shader>();
	_textureShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/floorTexture.vert");
	_textureShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/floorTexture.frag");
	_textureShader->CreateProgram();

	updatedTexture = false;
	isGrassInitialized = false;
	isPebbleInitialized = false;
	isDirtPebbleInitialized = false;

	_grassModel = std::make_unique<Model>("./Resources/Models/grass.fbx");
	_pebbleModel = std::make_unique<Model>("./Resources/Models/rock.fbx");
	_dirtPebbleModel = std::make_unique<Model>("./Resources/Models/dirt_pebble.fbx");

	_blendFloorModel = std::make_unique<Model>("./Resources/Models/round_floor.fbx");
	_blendShader = std::make_unique<Shader>();
	_blendShader->LoadFromFile(GL_VERTEX_SHADER, "./Resources/Shaders/basiclight.vert");
	_blendShader->LoadFromFile(GL_FRAGMENT_SHADER, "./Resources/Shaders/floorBlend.frag");
	_blendShader->CreateProgram();
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
		std::vector<bool> temp2(numTiles, false);
		_claimedMap = std::vector<std::vector<bool>>(numTiles, temp2);
	}

	_floorMap[(int)(state->pos.x)][(int)(state->pos.z)] = (FloorType)((int)(state->height));
	_claimedMap[(int)(state->pos.x)][(int)(state->pos.z)] = (bool)(state->pos.y);

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

	auto s = glm::scale(glm::mat4(1.0f), glm::vec3(tileScale, 1, tileScale));

	for (int x = 0; x < _floorMap.size(); x++) {
		for (int z = 0; z < _floorMap[0].size(); z++) {
			// skip the tile that is default
			if (_floorMap[x][z] == FLOOR_GRASS) {
				continue;
			};

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

	// Also draw the grass
	for (auto& grassM : _grassList)
	{
		_objectShader->set_uniform("u_model", grassM);
		_grassModel->render(_objectShader);
	}

	for (auto& pebbleM : _pebbleList)
	{
		_objectShader->set_uniform("u_model", pebbleM);
		_pebbleModel->render(_objectShader);
	}

	for (auto& dirtPebbleM : _dirtPebbleList)
	{
		_objectShader->set_uniform("u_model", dirtPebbleM);
		_dirtPebbleModel->render(_objectShader);
	}

	//floorMesh.Draw(_objectShader, fbo->getRGBA());

	_blendShader->Use();
	_blendShader->set_uniform("u_projection", camera->projection_matrix());
	_blendShader->set_uniform("u_view", camera->view_matrix());
	_blendShader->set_uniform("u_transparency", _state->transparency * _alpha);
	_blendShader->set_uniform("u_dirlight.direction", glm::vec3(0.0f, -1.0f, -0.4f));
	_blendShader->set_uniform("u_dirlight.ambient", glm::vec3(0.2f, 0.2f, 0.3f));
	_blendShader->set_uniform("u_dirlight.diffuse", glm::vec3(0.8f, 0.8f, 0.9f));
	_blendShader->set_uniform("u_numdirlights", static_cast<GLuint>(1));

	float map_radius = std::sqrtf(MAP_WIDTH * MAP_WIDTH * 2) + 2.0f;
	floorMesh = (static_cast<Model*>(_blendFloorModel.get()))->getMeshAt(0);

	for (int i = 0; i < MAP_BLEND_NUM; i++) {
		// Compute model matrix based on state: t * r * s
		auto t = glm::translate(glm::mat4(1.0f), _state->pos + glm::vec3(0, -0.002f * i, 0));
		auto r = glm::lookAt(glm::vec3(0.0f), _state->forward, _state->up);
		s = glm::scale(glm::mat4(1.0f), glm::vec3(map_radius + MAP_BLEND_DIST * i, 1, map_radius + MAP_BLEND_DIST * i));

		auto model = t * r * s;

		// Pass model matrix into shader
		_blendShader->set_uniform("u_model", model);

		_blendShader->set_uniform("blendRate", i / MAP_BLEND_NUM);

		floorMesh.Draw(_blendShader);
	}

}
void CFloorEntity::initGrass()
{
	if (isGrassInitialized) {
		return;
	}
	srand(GRASS_SEED);
	for (int x = 0; x < _floorMap.size(); x++) {
		for (int z = 0; z < _floorMap[0].size(); z++) {
			// skip the tile that is default
			if (_floorMap[x][z] == FLOOR_GRASS && !_claimedMap[x][z] && !(rand() % 5)) {
				// get actual position and scale of tile
				float xPos = ((float)x * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
				float zPos = ((float)z * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
				// slight randomize offset
				xPos += ((rand() % 101) - 50.0f) * (tileScale / 2) / 75;
				zPos += ((rand() % 101) - 50.0f) * (tileScale / 2) / 75;
				// randomize rotation
				float degree = rand() % 360;
				glm::vec3 forward = glm::rotate(glm::mat4(1), degree, glm::vec3(0, 1, 0)) * glm::vec4(_state->forward, 0);
				// randomize scale
				float scale = ((rand() % 50) + 50.0f) / 75;

				auto t = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, 0.002f, zPos));
				auto r = glm::lookAt(glm::vec3(0.0f), forward, glm::vec3(0, 1, 0));
				auto s = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

				_grassList.push_back(t * r * s);
			}
		}
	}
	isGrassInitialized = true;
}

void CFloorEntity::initPebble()
{
	if (isPebbleInitialized) {
		return;
	}
	srand(PEBBLE_SEED);
	for (int x = 0; x < _floorMap.size(); x++) {
		for (int z = 0; z < _floorMap[0].size(); z++) {
			// possible to have 2 pebbles in one tile
			for (int i = 0; i < 2; i++) {
				if (_floorMap[x][z] == FLOOR_GRASS && !_claimedMap[x][z] && !(rand() % 9)) {
					// get actual position and scale of tile
					float xPos = ((float)x * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
					float zPos = ((float)z * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
					// slight randomize offset
					xPos += ((rand() % 101) - 50.0f) * (tileScale / 2) / 75;
					zPos += ((rand() % 101) - 50.0f) * (tileScale / 2) / 75;
					// randomize rotation
					float degree = rand() % 360;
					glm::vec3 forward = glm::rotate(glm::mat4(1), degree, glm::vec3(0, 1, 0)) * glm::vec4(_state->forward, 0);
					// randomize scale
					float scale = ((rand() % 50) + 50.0f) / 75;

					auto t = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, 0.002f, zPos));
					auto r = glm::lookAt(glm::vec3(0.0f), forward, glm::vec3(0, 1, 0));
					auto s = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

					_pebbleList.push_back(t * r * s);
				}
			}

		}
	}
	isPebbleInitialized = true;
}

void CFloorEntity::initDirtPebble()
{
	if (isDirtPebbleInitialized) {
		return;
	}
	srand(DIRT_PEBBLE_SEED);
	for (int x = 0; x < _floorMap.size(); x++) {
		for (int z = 0; z < _floorMap[0].size(); z++) {
			// possible to have 2 pebbles in one tile
			for (int i = 0; i < 2; i++) {
				if (_floorMap[x][z] == FLOOR_DIRT && !_claimedMap[x][z] && !(rand() % 4)) {
					// get actual position and scale of tile
					float xPos = ((float)x * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
					float zPos = ((float)z * tileScale) - (MAP_WIDTH / 2) + tileScale / 2;
					// slight randomize offset
					xPos += ((rand() % 101) - 50.0f) * (tileScale / 2) / 75;
					zPos += ((rand() % 101) - 50.0f) * (tileScale / 2) / 75;
					// randomize rotation
					float degree = rand() % 360;
					glm::vec3 forward = glm::rotate(glm::mat4(1), degree, glm::vec3(0, 1, 0)) * glm::vec4(_state->forward, 0);
					// randomize scale
					float scale = ((rand() % 50) + 50.0f) / 75;

					auto t = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, 0.002f, zPos));
					auto r = glm::lookAt(glm::vec3(0.0f), forward, glm::vec3(0, 1, 0));
					auto s = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

					_dirtPebbleList.push_back(t * r * s);
				}
			}

		}
	}
	isDirtPebbleInitialized = true;
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

void CFloorEntity::reset()
{
	_grassList.clear();
	_pebbleList.clear();
	_dirtPebbleList.clear();
	isGrassInitialized = false;
	isPebbleInitialized = false;
	isDirtPebbleInitialized = false;
	_floorMap.clear();
	_claimedMap.clear();
}