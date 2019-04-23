/**
 * DirectionalLight.hpp
 */

#pragma once

#include "Light.hpp"

#include <glm/gtx/transform.hpp>

class DirectionalLight : public Light
{
public:
  DirectionalLight();
  DirectionalLight(const std::string &name, const LightIntensity &intensity, const glm::vec3 &direction);
  ~DirectionalLight();

  void setUniforms(std::unique_ptr<Shader> const &shader);
  void update();
  void draw(std::unique_ptr<Shader> const &shader);

  glm::vec3 direction() const;

  void set_name(const std::string &name);
  void set_direction(const glm::vec3 &direction);
  void set_direction(float x, float y, float z);

protected:
  // Uniform names
  std::string _name_direction;

  // Light properties
  glm::vec3 _direction;
};