/**
 * Model.cpp
 */

#include "Model.hpp"

#include <iostream>

Model::Model(const char *path)
{
  LoadModel(path);
}

Model::~Model()
{
  for (Texture t : _textures)
  {
    glDeleteTextures(1, &t.id);
    t.id = 0;
  }
}

void Model::Draw(std::unique_ptr<Shader> const &shader)
{
  for (Mesh mesh : _meshes) mesh.Draw(shader);
}

// Using Assimp, loads and post-processes model data such that all geometry
// is converted to triangles, texture coordinates are properly mapped, and
// tangent and bi-tangent vectors are calculated per vertex.
void Model::LoadModel(const std::string &path)
{
  Assimp::Importer importer;

  // Convert model data to triangles, flip texture coordinates on the y-axis
  // when necessary
  unsigned int post_process_flags = aiProcess_Triangulate | aiProcess_FlipUVs;
  const aiScene *scene = importer.ReadFile(path, post_process_flags);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    // Scene or rootnode of scene is NULL or the returned data is incomplete.
    std::cerr << "[ERROR] Assimp: " << importer.GetErrorString() << std::endl;
    return;
  }
  _directory = path.substr(0, path.find_last_of('/'));

  ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode *node, const aiScene *scene)
{
  // Process and keep track of all meshes in the aiNode.
  for (unsigned int mesh_idx = 0; mesh_idx < node->mNumMeshes; mesh_idx++)
  {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[mesh_idx]];
    _meshes.push_back(ProcessMesh(mesh, scene));
  }

  for (unsigned int child_idx = 0; child_idx < node->mNumChildren; child_idx++)
    ProcessNode(node->mChildren[child_idx], scene);
}

Mesh Model::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
  std::vector<Vertex>  vertices;
  std::vector<GLuint>  indices;
  std::vector<Texture> textures;

  for (unsigned int vert_idx = 0; vert_idx < mesh->mNumVertices; vert_idx++)
  {
    Vertex vertex;
    glm::vec3 vector3; // Placeholder vector to store position and normal data

    // Position
    vector3.x = mesh->mVertices[vert_idx].x;
    vector3.y = mesh->mVertices[vert_idx].y;
    vector3.z = mesh->mVertices[vert_idx].z;
    vertex.position = vector3;

    // Normal
    vector3.x = mesh->mNormals[vert_idx].x;
    vector3.y = mesh->mNormals[vert_idx].y;
    vector3.z = mesh->mNormals[vert_idx].z;
    vertex.normal = vector3;

    // UV
    if (mesh->mTextureCoords[0])
    {
      glm::vec2 coord;
      coord.x = mesh->mTextureCoords[0][vert_idx].x;
      coord.y = mesh->mTextureCoords[0][vert_idx].y;
      vertex.tex_coord = coord;
    }
    else
    {
      vertex.tex_coord = glm::vec2(0.0f);
    }

    vertices.push_back(vertex);
  }

  // Iterate over each face and their respective indices and store them into
  // the mesh's indices array.
  for (unsigned int face_idx = 0; face_idx < mesh->mNumFaces; face_idx++)
  {
    aiFace face = mesh->mFaces[face_idx];
    for (unsigned int i = 0; i < face.mNumIndices; i++)
      indices.push_back(face.mIndices[i]);
  }

  // Load material
  aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

  // Diffuse textures
  std::vector<Texture> diffuse_maps = LoadMaterialTextures(material,
    aiTextureType_DIFFUSE, TextureType::DIFFUSE);
  textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

  // Specular textures
  std::vector<Texture> specular_maps = LoadMaterialTextures(material,
    aiTextureType_SPECULAR, TextureType::SPECULAR);
  textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

  return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial *material,
  aiTextureType ai_tex_type, TextureType tex_type)
{
  std::vector<Texture> textures;

  // no texture
  if (material->GetTextureCount(ai_tex_type) == 0)
  {
	  // Create and store texture
	  Texture texture;
	  texture.id = LoadTextureFromFile("blank.jpg", "./Resources/Models");
	  texture.type = tex_type;
	  texture.path = "blank.jpg";

	  textures.push_back(texture);
	  _textures.push_back(texture);
  }

  for (unsigned int i = 0; i < material->GetTextureCount(ai_tex_type); i++)
  {
    aiString path;
    material->GetTexture(ai_tex_type, i, &path);
    bool loaded = false;

    for (unsigned int j = 0; j < _textures.size(); j++)
    {
      if (std::strcmp(_textures[j].path.c_str(), path.C_Str()) == 0)
      {
        // Texture was previously loaded
        textures.push_back(_textures[j]);
        loaded = true;
        break;
      }
    }

    if (!loaded)
    {
      // Create and store texture
      Texture texture;
      texture.id = LoadTextureFromFile(path.C_Str(), _directory);
      texture.type = tex_type;
      texture.path = path.C_Str();

      textures.push_back(texture);
      _textures.push_back(texture);
    }
  }

  return textures;
}