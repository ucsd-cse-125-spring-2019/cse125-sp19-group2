/**
 * PointLight.cpp
 */

#include "PointLight.hpp"

PointLight::PointLight()
{
}

PointLight::PointLight(const std::string & name, const LightIntensity & intensity, const glm::vec3 & position, const LightAttenuation & attenuation)
{
  set_name(name);
  set_intensity(intensity);
  set_position(position);
  set_attenuation(attenuation);
}

PointLight::~PointLight() {}

void PointLight::setUniforms(std::unique_ptr<Shader> const &shader)
{
  Light::setUniforms(shader);
  shader->set_uniform(_name_position.c_str(), _position);
  shader->set_uniform(_name_constant_attenuation.c_str(), _attenuation.constant);
  shader->set_uniform(_name_linear_attenuation.c_str(), _attenuation.linear);
  shader->set_uniform(_name_quadratic_attenuation.c_str(), _attenuation.quadratic);
}

void PointLight::update()
{
  _world_mtx = glm::translate(glm::mat4(1), _position);
}

void PointLight::draw(std::unique_ptr<Shader> const &shader)
{
  shader->set_uniform("u_model", _world_mtx);
}

glm::vec3 PointLight::position() const
{
  return _position;
}

void PointLight::set_name(const std::string & name)
{
  Light::set_name(name);
  _name_position = _name + ".position";
  _name_constant_attenuation  = _name + ".constant";
  _name_linear_attenuation    = _name + ".linear";
  _name_quadratic_attenuation = _name + ".quadratic";
}

void PointLight::set_position(const glm::vec3 &position)
{
  _position = position;
}

void PointLight::set_position(float x, float y, float z)
{
  _position = glm::vec3(x, y, z);
}

void PointLight::set_attenuation(const LightAttenuation & attenuation)
{
  _attenuation = attenuation;
}

void PointLight::set_constant_attenuation(GLfloat value)
{
  _attenuation.constant = value;
}

void PointLight::set_linear_attenuation(GLfloat value)
{
  _attenuation.linear = value;
}

void PointLight::set_quadratic_attenuation(GLfloat value)
{
  _attenuation.quadratic = value;
}
