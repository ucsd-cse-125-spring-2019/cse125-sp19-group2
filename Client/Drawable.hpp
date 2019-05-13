#pragma once

#include <memory>
#include "Shader.hpp"
#include "AnimatedMesh.hpp"

/**
  * Interface for drawable objects. Wraps animated objects and static models.
  */
class Drawable
{
public:
	virtual void render(std::unique_ptr<Shader> const& shader) = 0;

	// For animated objects only
	virtual void update() {};
};

