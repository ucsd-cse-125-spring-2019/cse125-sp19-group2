/**
 * DirectionalLight.cpp
 */

#include "DirectionalLight.hpp"

DirectionalLight::DirectionalLight() {}

DirectionalLight::DirectionalLight(const std::string & name, const LightIntensity & intensity, const glm::vec3 & direction)
{
  set_name(name);
  set_intensity(intensity);
  set_direction(direction);
}

DirectionalLight::~DirectionalLight() {}

void DirectionalLight::setUniforms(std::unique_ptr<Shader> const & shader)
{
  Light::setUniforms(shader);
  shader->set_uniform(_name_direction.c_str(), _direction);
}

void DirectionalLight::update() {}

void DirectionalLight::draw(std::unique_ptr<Shader> const & shader)
{
}

glm::vec3 DirectionalLight::direction() const
{
  return _direction;
}

void DirectionalLight::set_name(const std::string & name)
{
  Light::set_name(name);
  _name_direction = name + ".direction";
}

void DirectionalLight::set_direction(const glm::vec3 & direction)
{
  _direction = direction;
}

void DirectionalLight::set_direction(float x, float y, float z)
{
  _direction = glm::vec3(x, y, z);
}
