/**
 * Shader.hpp
 */

#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <initializer_list>

class Shader
{
public:

  Shader();
  ~Shader();

  void LoadFromFile(GLenum type, const char *path);
  void LoadFromText(GLenum type, const char *code);
  void CreateProgram();

  void Use() const;

  void set_uniform(const char *uniform, GLint value);
  void set_uniform(const char *uniform, GLuint value);

  void set_uniform(const char *uniform, GLfloat value);
  void set_uniform(const char *uniform, const glm::vec2 &values);
  void set_uniform(const char *uniform, const glm::vec3 &values);
  void set_uniform(const char *uniform, const glm::vec4 &values);

  void set_uniform(const char *uniform, const glm::mat3 &mat);
  void set_uniform(const char *uniform, const glm::mat4 &mat);
  void set_uniform(const char *uniform, const std::vector<glm::mat4> &mats,
    const GLuint count);

  void CleanUp();

  GLuint program();

private:

  GLuint _program;
  GLuint _vertex_shader;
  GLuint _fragment_shader;

  // Map of uniform names to locations
  std::unordered_map<std::string, GLint> _uniforms;

  /**
   * \brief Finds and stores the specified uniform variable's location in the shader program.
   */
  void RegisterUniform(const char* uniform);

  /**
   * \brief Automatically finds all uniform variables for the shader program.
   */
  void AutoRegisterUniforms();
};

#endif /* SHADER_HPP */
