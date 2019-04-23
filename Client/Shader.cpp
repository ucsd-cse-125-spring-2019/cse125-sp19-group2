/**
 * Shader.cpp
 */

#include "Shader.hpp"
#include "Shared/Logger.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

Shader::Shader() : _program(0), _vertex_shader(0), _fragment_shader(0) {
}

Shader::~Shader() {
  CleanUp();
}

void Shader::LoadFromFile(GLenum type, const char* path) {
  std::ifstream shader_file;

  // Ensure shader_file can throw an exception.
  shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    // Read shader code from specified path.
    shader_file.open(path);
    std::stringstream shader_string_stream;
    shader_string_stream << shader_file.rdbuf();
    shader_file.close();

    const std::string code = shader_string_stream.str();
    LoadFromText(type, code.c_str());
  }
  catch (std::ifstream::failure e) {
    Logger::getInstance()->error("Shader file could not be found at: " +
      std::string(path));
  }
}

void Shader::LoadFromText(GLenum type, const char* code) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &code, nullptr);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint info_log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);
    GLchar* info_log = new GLchar[info_log_len];
    glGetShaderInfoLog(shader, info_log_len, nullptr, info_log);
    Logger::getInstance()->error("Shader compilation failed");
    delete[] info_log;
  }

  if (type == GL_VERTEX_SHADER) {
    _vertex_shader = shader;
  }
  else if (type == GL_FRAGMENT_SHADER) {
    _fragment_shader = shader;
  }
  else {
    Logger::getInstance()->error("Unsupported shader or invalid shader type");
  }
}

void Shader::CreateProgram() {
  // Create program, attach shaders, and link program
  GLuint program = glCreateProgram();

  if (_vertex_shader != 0)
    glAttachShader(program, _vertex_shader);
  if (_fragment_shader != 0)
    glAttachShader(program, _fragment_shader);

  glLinkProgram(program);

  // Check for successful program linkage
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    GLint info_log_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_len);
    GLchar* info_log = new GLchar[info_log_len];
    glGetProgramInfoLog(program, info_log_len, nullptr, info_log);
    Logger::getInstance()->error("Shader program linking failed");
    delete[] info_log;
  }

  // Detach shaders
  glDetachShader(program, _vertex_shader);
  glDetachShader(program, _fragment_shader);
  glDeleteShader(_vertex_shader);
  glDeleteShader(_fragment_shader);
  _vertex_shader = 0;
  _fragment_shader = 0;

  _program = program;

  // Register uniform variables
  AutoRegisterUniforms();
}

void Shader::Use() const {
  glUseProgram(_program);
}

void Shader::set_uniform(const char* uniform, GLint value) {
  auto u_elem = _uniforms.find(uniform);
  if (u_elem == _uniforms.end()) {
    Logger::getInstance()->error(
      "Attempting to set unregistered uniform: " +
      std::string(uniform));
    return;
  }
  glUniform1iv(_uniforms.at(uniform), 1, &value);
}

void Shader::set_uniform(const char* uniform, GLuint value) {
  auto u_elem = _uniforms.find(uniform);
  if (u_elem == _uniforms.end()) {
    Logger::getInstance()->error(
      "Attempting to set unregistered uniform: " +
      std::string(uniform));
    return;
  }
  glUniform1uiv(_uniforms.at(uniform), 1, &value);
}

void Shader::set_uniform(const char* uniform, GLfloat value) {
  auto u_elem = _uniforms.find(uniform);
  if (u_elem == _uniforms.end()) {
    Logger::getInstance()->error(
      "Attempting to set unregistered uniform: " +
      std::string(uniform));
    return;
  }
  glUniform1fv(_uniforms.at(uniform), 1, &value);
}

void Shader::set_uniform(const char* uniform, const glm::vec2& values) {
  auto u_elem = _uniforms.find(uniform);
  if (u_elem == _uniforms.end()) {
    Logger::getInstance()->error(
      "Attempting to set unregistered uniform: " +
      std::string(uniform));
    return;
  }
  glUniform2fv(_uniforms.at(uniform), 1, &values[0]);
}

void Shader::set_uniform(const char* uniform, const glm::vec3& values) {
  auto u_elem = _uniforms.find(uniform);
  if (u_elem == _uniforms.end()) {
    Logger::getInstance()->error(
      "Attempting to set unregistered uniform: " +
      std::string(uniform));
    return;
  }
  glUniform3fv(_uniforms.at(uniform), 1, &values[0]);
}

void Shader::set_uniform(const char* uniform, const glm::vec4& values) {
  auto u_elem = _uniforms.find(uniform);
  if (u_elem == _uniforms.end()) {
    Logger::getInstance()->error(
      "Attempting to set unregistered uniform: " +
      std::string(uniform));
    return;
  }
  glUniform4fv(_uniforms.at(uniform), 1, &values[0]);
}

void Shader::set_uniform(const char* uniform, const glm::mat3& mat) {
  auto u_elem = _uniforms.find(uniform);
  if (u_elem == _uniforms.end()) {
    Logger::getInstance()->error(
      "Attempting to set unregistered uniform: " +
      std::string(uniform));
    return;
  }
  glUniformMatrix3fv(_uniforms.at(uniform), 1, GL_FALSE, &mat[0][0]);
}

void Shader::set_uniform(const char* uniform, const glm::mat4& mat) {
  auto u_elem = _uniforms.find(uniform);
  if (u_elem == _uniforms.end()) {
    Logger::getInstance()->error(
      "Attempting to set unregistered uniform: " +
      std::string(uniform));
    return;
  }
  glUniformMatrix4fv(_uniforms.at(uniform), 1, GL_FALSE, &mat[0][0]);
}

void Shader::set_uniform(const char* uniform, const std::vector<glm::mat4>& mats,
  const GLuint count) {
  auto u_elem = _uniforms.find(uniform);
  if (u_elem == _uniforms.end()) {
    Logger::getInstance()->error(
      "Attempting to set unregistered uniform: " +
      std::string(uniform));
    return;
  }
  glUniformMatrix4fv(_uniforms.at(uniform), count, GL_FALSE, &mats[0][0][0]);
}

void Shader::CleanUp() {
  glDeleteProgram(_program);
  glDeleteShader(_vertex_shader);
  glDeleteShader(_fragment_shader);
  _program = 0;
  _vertex_shader = 0;
  _fragment_shader = 0;
}

GLuint Shader::program() {
  return _program;
}

void Shader::RegisterUniform(const char* uniform) {
  GLint uniform_loc = glGetUniformLocation(_program, static_cast<const GLchar *>(uniform));

  if (uniform_loc == -1)
  {
    Logger::getInstance()->error("For program " + std::to_string(_program) + ": \""
      + std::string(uniform)
      + "\" does not correspond to an active uniform variable OR"
      + " start with reserved prefix \"gl_\" OR"
      + "is associated with an atomic counter or a named uniform block.");
  }
  else
  {
    _uniforms.emplace(std::string(uniform), uniform_loc);
    Logger::getInstance()->info("For program " + std::to_string(_program) + ": \""
      + std::string(uniform) + "\" registered.");
  }
}

void Shader::AutoRegisterUniforms()
{
  GLint uniform_name_max_len;
  GLint num_uniforms;

  // Get max uniform name length and number of uniforms for shader program
  glGetProgramiv(_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_name_max_len);
  glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &num_uniforms);
  Logger::getInstance()->info("For program " + std::to_string(_program) + ": "
    + std::to_string(num_uniforms) + " uniforms.");

  // Uniform info
  GLsizei length;
  GLint size;
  GLenum type;
  std::vector<GLchar>name(uniform_name_max_len, 0);

  // Get all uniforms
  for (GLint i = 0; i < num_uniforms; i++)
  {
    glGetActiveUniform(_program, static_cast<GLuint>(i), uniform_name_max_len,
      &length, &size, &type, name.data());
    RegisterUniform(name.data());
  }
}
