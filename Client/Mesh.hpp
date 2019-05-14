/**
 * Mesh.hpp
 */

#ifndef MESH_HPP
#define MESH_HPP

#include "Vertex.hpp"
#include "Texture.hpp"
#include "Shader.hpp"

#include <glad/glad.h>

#include <vector>
#include <memory>

class Mesh
{
public:
  // Creates a mesh with given set of vertices, indices, and textures.
  Mesh(const std::vector<Vertex>  &vertices,
    const std::vector<GLuint>  &indices,
    const std::vector<Texture> &textures);

  // Renders the mesh to the current framebuffer.
  void Draw(std::unique_ptr<Shader> const &shader);

  // Renders the mesh to the current framebuffer given texture ID.
  void Draw(std::unique_ptr<Shader> const &shader, GLuint textureID);

  // Renders the mesh into lines to the current framebuffer
  void DrawLine(std::unique_ptr<Shader> const &shader);

private:

  std::vector<Vertex>  _vertices;
  std::vector<GLuint>  _indices;
  std::vector<Texture> _textures;

  GLuint _vao, _vbo, _ebo;

  // Stores all relevant information about the mesh.
  void Setup();
};

#endif /* MESH_HPP */
