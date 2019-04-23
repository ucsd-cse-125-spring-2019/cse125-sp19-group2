/**
 * Light.hpp
 */

#pragma once

#include "Shader.hpp"
#include <memory>
#include <string>

struct LightIntensity
{
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
};

struct LightAttenuation
{
  GLfloat constant;
  GLfloat linear;
  GLfloat quadratic;
};

class Light
{
public:
  Light() = default;
  ~Light() = default;

  virtual void setUniforms(std::unique_ptr<Shader> const &shader);
  virtual void update();

  /**
   * \brief Draws a representation of the light within the scene. For debugging purposes.
   * \param shader(std::unique_ptr<Shader>const&) The shader program to render the light.
   */
  virtual void draw(std::unique_ptr<Shader> const &shader);

  /**
  * \brief Sets the uniform names for the light.
  * \param name(std::string) The uniform name of the light.
  */
  virtual void set_name(const std::string &name);
  void set_intensity(const LightIntensity &intensity);
  void set_ambient(float r, float g, float b);
  void set_diffuse(float r, float g, float b);
  void set_specular(float r, float g, float b);

protected:

  // Uniform names
  std::string _name;
  std::string _name_ambient;
  std::string _name_diffuse;
  std::string _name_specular;

  // Light properties
  LightIntensity _intensity;
  
  // Rendering information (for debugging purposes)
  glm::mat4 _world_mtx;
};