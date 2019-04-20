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
  GLuint program = glCreateProgram();

  if (_vertex_shader != 0)
    glAttachShader(program, _vertex_shader);
  if (_fragment_shader != 0)
    glAttachShader(program, _fragment_shader);

  glLinkProgram(program);

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

  glDetachShader(program, _vertex_shader);
  glDetachShader(program, _fragment_shader);
  glDeleteShader(_vertex_shader);
  glDeleteShader(_fragment_shader);
  _vertex_shader = 0;
  _fragment_shader = 0;

  _program = program;
}

void Shader::Use() const {
  glUseProgram(_program);
}

void Shader::RegisterUniform(const char* uniform) {
  GLuint uniform_loc = glGetUniformLocation(_program, uniform);
  _uniforms[uniform] = uniform_loc;
}

void Shader::RegisterUniformList(std::initializer_list<const char *> uniforms) {
  for (const char* uniform : uniforms) RegisterUniform(uniform);
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
