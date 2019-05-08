#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "Shared/Common.hpp"
#include <unordered_map>
#include "Camera.hpp"
#include "Shared/BaseState.hpp"
#include "Model.hpp"

/**
 * \struct contain all information of a collision box/sphere
 */
struct colliderInfo {
	glm::vec3 pos;		// Position of collision box
	glm::vec3 scale;
	ColliderType colliderType;
};

/**
 * \ vertices of cube (it is dirty I know)
 */
const GLfloat cubeVertices[] = {
	-0.5f, 0, 0.5f,
	0.5f, 0, 0.5f,
	0.5f, 0, -0.5f,
	-0.5f, 0, -0.5f,
	-0.5f, 1.0f, 0.5f,
	0.5f, 1.0f, 0.5f,
	0.5f, 1.0f, -0.5f,
	-0.5f, 1.0f, -0.5f
};

/**
* \ indices of cube vertices in order (it is dirty I know)
*/
const GLuint cubeIndices[] = {
	0,1, 1,2, 2,3, 3,0,
	4,5, 5,6, 6,7, 7,4,
	0,4, 1,5, 2,6, 3,7
};

/**
 * \brief Manage all collision boxes' updating and rendering (mainly just for rendering in debug mode).
 */
class ColliderManager
{
private:
	ColliderManager();
	std::unordered_map<uint32_t, std::shared_ptr<colliderInfo>> colliderList;
	static std::unique_ptr<Model> _cubeModel;
	static std::unique_ptr<Model> _cylinderModel;
	static std::unique_ptr<Shader> _shader;
public:
	/**
	* \brief The singleton getter of ColliderManager (create one if not exist)
	* \return ColliderManager&: An ColliderManager Object
	*/
	static ColliderManager& getInstance();

	/**
	* \brief Dispatch new state to the entity to update collision box
	* \param state(std::shared_ptr<BaseState> const&) Update state
	*/
	void updateState(std::shared_ptr<BaseState> const& state);

	/**
	* \brief Remove collision box information of given id
	* \param id(uint32_t) id of entity
	*/
	void erase(uint32_t id);

	/**
	* \brief Render all collision boxes in the game world
	* \param camera(std::unique_ptr<Camera> const&) Local camera
	*/
	void render(std::unique_ptr<Camera> const& camera);

	/**
	* \brief Helper function for initializing cube object
	*/
	void initCube();
};

