/**
 * Model.hpp
 */

#ifndef MODEL_HPP
#define MODEL_HPP

#include "mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

class Model
{
public:

  // Creates a model from a file.
  Model(const char *path);

  ~Model();

  void Draw(std::unique_ptr<Shader> const &shader);

  // Get mesh in _meshes at index i
  Mesh getMeshAt(int i);

private:

  // Mesh Data
  std::vector<Texture> _textures;
  std::vector<Mesh>    _meshes;
  std::string          _directory;

  void LoadModel(const std::string &path);

  // Processes model tree data from assimp.
  void ProcessNode(aiNode *node, const aiScene *scene);

  // Processes mesh data from assimp.
  Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

  // Loads texture of given type for given material.
  std::vector<Texture> LoadMaterialTextures(aiMaterial *material,
    aiTextureType ai_tex_type, TextureType tex_type);
};

#endif /* MODEL_HPP */
