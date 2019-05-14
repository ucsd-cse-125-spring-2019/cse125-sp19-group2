/**
 * Mesh.cpp
 */

#include "Mesh.hpp"

#include <iostream>

Mesh::Mesh(const std::vector<Vertex>  &vertices,
  const std::vector<GLuint>  &indices,
  const std::vector<Texture> &textures)
{
  _vertices = vertices;
  _indices = indices;
  _textures = textures;

  Setup();
}

void Mesh::Draw(std::unique_ptr<Shader> const &shader)
{
  // Bind textures (last texture of specified type is used)
  for (GLuint i = 0; i < _textures.size(); i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);

    TextureType tex_type = _textures[i].type;
    if (tex_type == TextureType::DIFFUSE)
      shader->set_uniform("u_material.diffuse", i);
    else if (tex_type == TextureType::SPECULAR)
      shader->set_uniform("u_material.specular", i);

    glBindTexture(GL_TEXTURE_2D, _textures[i].id);
  }

  glBindVertexArray(_vao);
  glDrawElements(GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}

void Mesh::Draw(std::unique_ptr<Shader> const &shader, GLuint textureID)
{
	// Bind given texture
	glActiveTexture(GL_TEXTURE0);
	shader->set_uniform("u_material.diffuse", 0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::DrawLine(std::unique_ptr<Shader> const &shader) {
	glBindVertexArray(_vao);
	glEnable(GL_LINE_SMOOTH);
	glDrawElements(GL_LINE_STRIP, (GLsizei)_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

// Binds mesh's VAO, VBO, and EBO data to corresponding buffers and sets the
// attribute locations of position, normal, and texture coordinate data to
// location 0, 1, and 2, respectively.
void Mesh::Setup()
{
  glGenVertexArrays(1, &_vao);
  glGenBuffers(1, &_vbo);
  glGenBuffers(1, &_ebo);

  glBindVertexArray(_vao);

  // Bind vertex data to the mesh's VBO.
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex),
    &_vertices[0], GL_STATIC_DRAW);

  // Bind the index information to the mesh's EBO.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint),
    &_indices[0], GL_STATIC_DRAW);

  // Set vertex attribute location 0 to vertex positions.
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

  // Set vertex attribute location 1 to vertex normals.
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    (GLvoid*)offsetof(Vertex, normal));

  // Set vertex attribute location 2 to vertex texture coordinates.
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    (GLvoid*)offsetof(Vertex, tex_coord));

  glBindVertexArray(0);
}
