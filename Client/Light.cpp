/**
 * Light.cpp
 */

#include "Light.hpp"

void Light::setUniforms(std::unique_ptr<Shader> const &shader)
{
  shader->set_uniform(_name_ambient.c_str(),  _intensity.ambient);
  shader->set_uniform(_name_diffuse.c_str(),  _intensity.diffuse);
  shader->set_uniform(_name_specular.c_str(), _intensity.specular);
}

void Light::update() {}

void Light::draw(std::unique_ptr<Shader> const &shader) {}

void Light::set_name(const std::string &name)
{
  _name = name;
  _name_ambient  = _name + ".ambient";
  _name_diffuse  = _name + ".diffuse";
  _name_specular = _name + ".specular";
}

void Light::set_intensity(const LightIntensity & intensity)
{
  _intensity = intensity;
}

void Light::set_ambient(float r, float g, float b)
{
  _intensity.ambient = glm::vec3(r, g, b);
}

void Light::set_diffuse(float r, float g, float b)
{
  _intensity.diffuse = glm::vec3(r, g, b);
}

void Light::set_specular(float r, float g, float b)
{
  _intensity.specular = glm::vec3(r, g, b);
}
