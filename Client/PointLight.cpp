/**
 * PointLight.cpp
 */

#include "PointLight.hpp"

static const GLfloat quad_vertex_data[] = {0.0f, 0.0f, 0.0f};

PointLight::PointLight()
{
}

PointLight::PointLight(const std::string & name, const LightIntensity & intensity, const glm::vec3 & position, const LightAttenuation & attenuation)
{
  set_name(name);
  set_intensity(intensity);
  set_position(position);
  set_attenuation(attenuation);

  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);
  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_data), quad_vertex_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
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

  glBindVertexArray(_vao);
  glPointSize(5.0f);
  glDrawArrays(GL_POINTS, 0, 1);
  glBindVertexArray(0);
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
