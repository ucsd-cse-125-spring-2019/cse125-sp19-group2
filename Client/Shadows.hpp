#pragma once

#include "FrameBuffer.h"
#include "Camera.hpp"
#include "Shader.hpp"

class Shadows
{
public:

	Shadows();
	Shadows(int width, int height, const glm::vec3 &light_pos, const glm::vec3 &light_dir);
	~Shadows();

	void process(GLsizei width, GLsizei height);

private:

	std::unique_ptr<FrameBuffer> _pass_shadow_map;
	std::unique_ptr<Camera> _light_camera;
	std::unique_ptr<Shader> _shadow_map_program;

	GLsizei _map_width, _map_height;
};                