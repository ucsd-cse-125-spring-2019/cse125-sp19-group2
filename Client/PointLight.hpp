/**
 * PointLight.hpp
 */

#ifndef POINT_LIGHT_HPP
#define POINT_LIGHT_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <string>
#include "Light.hpp"
#include "Shader.hpp"

struct LightAttenuation
{
  GLfloat constant;
  GLfloat linear;
  GLfloat quadratic;
};

class PointLight : public Light
{
 public:
  PointLight();
  PointLight(const std::string &name, const LightIntensity &intensity, const glm::vec3 &position, const LightAttenuation &attenuation);
  ~PointLight();
  
  void setUniforms(std::unique_ptr<Shader> const &shader);
  void update();
  void draw(std::unique_ptr<Shader> const &shader);
  
  glm::vec3 position() const;
 
  void set_name(const std::string & name);
  void set_position(const glm::vec3 & position);
  void set_position(float x, float y, float z);
  void set_attenuation(const LightAttenuation &attenuation);
  void set_constant_attenuation(GLfloat value);
  void set_linear_attenuation(GLfloat value);
  void set_quadratic_attenuation(GLfloat value);

 protected:
  // Uniform names
  std::string _name_position;
  std::string _name_constant_attenuation;
  std::string _name_linear_attenuation;
  std::string _name_quadratic_attenuation;
  
  // Light properties
  glm::vec3 _position;
  LightAttenuation _attenuation;
};

#endif /* POINT_LIGHT_HPP */
