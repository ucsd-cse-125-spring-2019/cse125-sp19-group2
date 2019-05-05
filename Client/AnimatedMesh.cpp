#include "AnimatedMesh.hpp"
#include "Shared/Logger.hpp"
#include <execution>
#include <optional>
#include <assimp/postprocess.h>
#include <fstream>
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define POSITION_LOCATION 0
#define NORMAL_LOCATION 1
#define TEXCOORD_LOCATION 2
#define BONEID_LOCATION 3
#define BONEID2_LOCATION 4
#define BONEWEIGHT_LOCATION 5
#define BONEWEIGHT2_LOCATION 6

#undef USE_FBX
#define USE_TAKE_INFO

using namespace std;
using namespace glm;

static inline mat4 mat4_cast(const aiMatrix4x4& m) { return transpose(make_mat4(&m.a1)); }

void WeightData::addWeight(uint32_t boneID, float weight) {
    for (uint32_t i = 0; i < MAX_BONES_PER_VERTEX; i++) {
        if (weights[i] == 0.0) {
            ids[i] = boneID;
            weights[i] = weight;
            return;
        }
    }

    // should never get here - more bones than we have space for
    Logger::getInstance()->warn("more bones than we have space for");
}

AnimatedMesh::AnimatedMesh(): _takeIndex(0), _VAO(0) {
    memset(_buffers, 0, 5);
}

AnimatedMesh::~AnimatedMesh() {
    for (auto& texture : _textures) {
        if (texture) {
            delete texture;
            texture = nullptr;
        }
    }

    if (_buffers[0] != 0) {
        glDeleteBuffers(5, _buffers);
    }

    if (_VAO != 0) {
        glDeleteVertexArrays(1, &_VAO);
        _VAO = 0;
    }
}

bool AnimatedMesh::loadMesh(const string& filename) {
    // Create the VAO
    glGenVertexArrays(1, &_VAO);
    glBindVertexArray(_VAO);

    // Create the buffers for the vertices attributes
    glGenBuffers(5, _buffers);

    bool ret = false;
    Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    //importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_STRICT_MODE, true);
    
    auto scene = importer.ReadFile(filename.c_str(),
                                         aiProcess_Triangulate |
                                         aiProcess_GenSmoothNormals |
                                         aiProcess_FlipUVs |
                                         aiProcess_JoinIdenticalVertices 
	                                    );

    if (scene) {
        _globalInverseTransform = mat4_cast(scene->mRootNode->mTransformation);
        _globalInverseTransform = inverse(_globalInverseTransform);
        ret = initScene(scene, filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", filename.c_str(), importer.GetErrorString());
    }

#ifdef USE_TAKE_INFO
	loadTakes(filename);
#endif 

    glBindVertexArray(0);

    importer.FreeScene();

    return ret;
}

void AnimatedMesh::render(const std::unique_ptr<Shader>& shader) {
    glBindVertexArray(_VAO);

    for (uint32_t i = 0; i < _entries.size(); i++) {
        const uint32_t materialIndex = _entries[i]._textureIndex;

        assert(materialIndex < _textures.size());

        if (_textures[materialIndex]) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _textures[materialIndex]->id);
            shader->set_uniform("u_material.diffuse", i);
        }

        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 _entries[i]._numIndices,
                                 GL_UNSIGNED_INT,
                                 (void*)(sizeof(uint32_t) * _entries[i]._baseIndex),
                                 _entries[i]._baseVertex);
    }
  
    glBindVertexArray(0);
}

void AnimatedMesh::getTransform(float second, vector<mat4>& transforms) {

    const float tickrate = _takes[_takeIndex]->tickrate != 0 ? _takes[_takeIndex]->tickrate : 30.0f;
    const float ticks = second * tickrate;
    const float time = fmod(ticks, _takes[_takeIndex]->duration);

    computeWorldMatrix(time, _root.get(), glm::mat4(1.0f));

    transforms.resize(_boneInfo.size());

    for (uint32_t i = 0; i < transforms.size(); i++) {
        transforms[i] = _boneInfo[i].worldMatrix;
    }
}

uint32_t AnimatedMesh::takeCount() const {
    return _takes.size();
}

std::string AnimatedMesh::getCurrentAnimName() const {
    return _takes[_takeIndex]->takeName;
}

void AnimatedMesh::loadBones(uint32_t meshIndex, const aiMesh* mesh, vector<WeightData>& bones) {
    for (uint32_t i = 0; i < mesh->mNumBones; i++) {
        uint32_t boneIndex = 0;
        string name(mesh->mBones[i]->mName.data);

        if (_boneMap.find(name) == _boneMap.end()) {
			BoneData bi;
			boneIndex = _boneMap[name] = _boneInfo.size();
            _boneInfo.emplace_back(bi).bindingMatrix = mat4_cast(mesh->mBones[i]->mOffsetMatrix);
        }
        else {
            boneIndex = _boneMap[name];
        }

        for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
            const uint32_t index = _entries[meshIndex]._baseVertex + mesh->mBones[i]->mWeights[j].mVertexId;
            const float weight = mesh->mBones[i]->mWeights[j].mWeight;
            bones[index].addWeight(boneIndex, weight);
        }
    }
}

void AnimatedMesh::initMesh(uint32_t meshIndex,
                            const aiMesh* paiMesh,
                            vector<vec3>& positions,
                            vector<vec3>& normals,
                            vector<vec2>& texCoords,
                            vector<WeightData>& bones,
                            vector<uint32_t>& indices) {
    const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

    // Populate the vertex attribute vectors
    for (uint32_t i = 0; i < paiMesh->mNumVertices; i++) {
        const aiVector3D* pPos = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &zero3D;

        positions.emplace_back(pPos->x, pPos->y, pPos->z);
        normals.emplace_back(pNormal->x, pNormal->y, pNormal->z);
        texCoords.emplace_back(pTexCoord->x, pTexCoord->y);
    }

    loadBones(meshIndex, paiMesh, bones);

    // Populate the index buffer
    for (uint32_t i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& face = paiMesh->mFaces[i];
        assert(face.mNumIndices == 3);
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }
}

bool AnimatedMesh::initMaterials(const aiScene* scene, const string& filename) {
    // Extract the directory part from the file name
    string::size_type slashIndex = filename.find_last_of("/");
    string dir;

    if (slashIndex == string::npos) {
        dir = ".";
    }
    else if (slashIndex == 0) {
        dir = "/";
    }
    else {
        dir = filename.substr(0, slashIndex);
    }

    bool ret = true;

    // Initialize the materials
    for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
        const aiMaterial* pMaterial = scene->mMaterials[i];

        _textures[i] = NULL;

        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                string p(path.data);

                if (p.substr(0, 2) == ".\\") {
                    p = p.substr(2, p.size() - 2);
                }

                string fullPath = dir + "/" + p;

                // Create and store texture
                Texture* texture = new Texture();
                texture->id = LoadTextureFromFile(path.C_Str(), "./Resources/Textures");
                texture->type = TextureType::DIFFUSE;
                texture->path = path.C_Str();

                _textures[i] = texture;

            }
        }
        else {
            // Create and store texture
            Texture* texture = new Texture();
            texture->id = LoadTextureFromFile("blank.jpg", "./Resources/Models");
            texture->type = TextureType::DIFFUSE;
            texture->path = "blank.jpg";

            _textures[i] = texture;
        }
    }

    return ret;
}

bool AnimatedMesh::initScene(const aiScene* scene, const string& filename) {
    _entries.resize(scene->mNumMeshes);
    _textures.resize(scene->mNumMaterials);

    // Fill up takes
    for (uint32_t i = 0; i < scene->mNumAnimations; i ++) {
		if (scene->mAnimations[i]->mDuration > 0) {
			_takes.push_back(initTake(scene->mAnimations[i]));
		}
    }

    // Fill up node list (recursively)
    _root = initNode(nullptr, scene->mRootNode);

    vector<glm::vec3> positions;
    vector<glm::vec3> normals;
    vector<glm::vec2> texCoords;
    vector<WeightData> bones;
    vector<uint32_t> indices;

    uint32_t numVertices = 0;
    uint32_t numIndices = 0;

    // Count the number of vertices and indices
    for (uint32_t i = 0; i < _entries.size(); i++) {
        _entries[i]._textureIndex = scene->mMeshes[i]->mMaterialIndex;
        _entries[i]._numIndices = scene->mMeshes[i]->mNumFaces * 3;
        _entries[i]._baseVertex = numVertices;
        _entries[i]._baseIndex = numIndices;

        numVertices += scene->mMeshes[i]->mNumVertices;
        numIndices += _entries[i]._numIndices;
    }

    // Reserve space in the vectors for the vertex attributes and indices
    positions.reserve(numVertices);
    normals.reserve(numVertices);
    texCoords.reserve(numVertices);
    bones.resize(numVertices);
    indices.reserve(numIndices);

    // Initialize the meshes in the scene one by one
    for (uint32_t i = 0; i < _entries.size(); i++) {
        const aiMesh* paiMesh = scene->mMeshes[i];
        initMesh(i, paiMesh, positions, normals, texCoords, bones, indices);
    }

    if (!initMaterials(scene, filename)) {
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _buffers[POSITION]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), &positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, _buffers[NORMAL]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, _buffers[TEXCOORD]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEXCOORD_LOCATION);
    glVertexAttribPointer(TEXCOORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, _buffers[BONE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bones[0]) * bones.size(), &bones[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(BONEID_LOCATION);
    glVertexAttribIPointer(BONEID_LOCATION, 4, GL_INT, sizeof(WeightData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONEID2_LOCATION);
    glVertexAttribIPointer(BONEID2_LOCATION, 4, GL_INT, sizeof(WeightData), (const GLvoid*)16);
    glEnableVertexAttribArray(BONEWEIGHT_LOCATION);
    glVertexAttribPointer(BONEWEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(WeightData), (const GLvoid*)32);
    glEnableVertexAttribArray(BONEWEIGHT2_LOCATION);
    glVertexAttribPointer(BONEWEIGHT2_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(WeightData), (const GLvoid*)48);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[INDEX]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    return glGetError() == GL_NO_ERROR;
}

unique_ptr<Channel> AnimatedMesh::initChannel(const aiNodeAnim* node) {
    auto cha = make_unique<Channel>();

    // Get joint name
    cha->jointName = node->mNodeName.data;

    // Fill in the keyframes value
    for (uint32_t i = 0; i < node->mNumPositionKeys; i++) {
        auto& key = node->mPositionKeys[i];
        auto time = float(key.mTime);
        vec4 value = vec4(key.mValue.x, key.mValue.y, key.mValue.z, 1);
        cha->translations.push_back(make_unique<Keyframe>(time, value));
    }

    for (uint32_t i = 0; i < node->mNumRotationKeys; i++) {
        auto& key = node->mRotationKeys[i];
        auto time = float(key.mTime);
        // quat and vec4 both internally use ordering x,y,z,w
        vec4 value = vec4(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w);;
        cha->rotations.push_back(make_unique<Keyframe>(time, value));
    }

    for (uint32_t i = 0; i < node->mNumScalingKeys; i++) {
        auto& key = node->mScalingKeys[i];
        auto time = float(key.mTime);
        vec4 value = vec4(key.mValue.x, key.mValue.y, key.mValue.z, 1);
        cha->scales.push_back(make_unique<Keyframe>(time, value));
    }

    // Transfer channel ptr back the parent 
    return cha;
}

unique_ptr<Node> AnimatedMesh::initNode(const Node* parent, const aiNode* node) {
    auto n = make_unique<Node>();

    // Fill in data
    n->name = node->mName.data;

#ifdef USE_FBX
	if(n->name == "") {
		node->mChildren[0]->mTransformation = node->mTransformation;
		return initNode(parent, node->mChildren[0]);
	}
#endif

    n->transform = mat4_cast(node->mTransformation);

    // Bookkeeping parent ptr
    n->parent = parent;

    // Populate children
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        n->children.push_back(initNode(n.get(), node->mChildren[i]));
    }

    // Bookkeeping
	_nodeList.push_back(n.get());

    // Transfer node ptr back to parent
    return n;
}

unique_ptr<Take> AnimatedMesh::initTake(const aiAnimation* animation) {

    auto take = make_unique<Take>();
    // Fill in metadata
    take->tickrate = float(animation->mTicksPerSecond);
    take->duration = float(animation->mDuration);

    // Init Channel
    for (uint32_t i = 0; i < animation->mNumChannels; i ++) {
		take->channels.push_back(initChannel(animation->mChannels[i]));
    }

    // Build channel map
#ifdef USE_FBX
	processFBXAnimation(*take);
#else
    for(uint32_t i = 0; i < take->channels.size(); i ++) {
		auto cha = take->channels[i].get();
		take->channelMap.insert({ cha->jointName, cha });
    }
#endif
    // Transfer take ptr back to parent
    return take;
}

const Channel* AnimatedMesh::findChannel(const string& nodeName) {
    auto& channels = _takes[_takeIndex]->channelMap;
    const auto cha = channels.find(nodeName);

    if (cha != channels.end()) {
        return (cha->second);
    }
    return nullptr;
}

uint32_t AnimatedMesh::findKeyframe(float time, const vector<unique_ptr<Keyframe>>& keyframes) {
    if (keyframes.size() <= 0) {
        Logger::getInstance()->warn("findKeyframe: size <= 0");
    }
    for (uint32_t i = 0; i < keyframes.size() - 1; i++) {
        if (time < keyframes[i + 1]->time) {
            return i;
        }
    }
    return keyframes.size() - 1;
}

std::tuple<uint32_t, uint32_t, uint32_t> AnimatedMesh::findFrame(float time, const Channel& channel) {
    uint32_t t = 0, r = 0, s = 0;
    t = findKeyframe(time, channel.translations);
    r = findKeyframe(time, channel.rotations);
    s = findKeyframe(time, channel.scales);
    return {t, r, s};
}

vec4 AnimatedMesh::getInterpolatedValue(float time, uint32_t start,
                                        const vector<unique_ptr<Keyframe>>& keyframes,
                                        std::function<glm::vec4(float, glm::vec4, glm::vec4)> interpolateFunction) {

    if(keyframes.size()==1) {
        // Only one frame exists, no need for interpolation
		return keyframes[0]->value;
    }

    auto& k1 = keyframes[start];
    uint32_t end = start + 1;
    if (start == keyframes.size() - 1) {
        end = 0;
    }
    auto& k2 = keyframes[end];

    const float startTime = k1->time;
    // Assuming take starts from 0, really
    const float endTime = end == 0 ? _takes[_takeIndex]->duration : k2->time;

    const float dt = endTime - startTime;
    float factor = (time - startTime) / dt;
    const auto& startValue = k1->value;
    const auto& endValue = k2->value;

    // Use interpolating function to get value
    return interpolateFunction(factor, startValue, endValue);
}

void AnimatedMesh::computeWorldMatrix(float time, const Node* node, const mat4& parent) {
    static auto vec3Interp = [](float f, vec4 v1, vec4 v2)
    {
        return v1 + f * (v2 - v1);
    };

    static auto quatInterp = [](float f, vec4 v1, vec4 v2)
    {
        return make_vec4(value_ptr(slerp(make_quat(value_ptr(v1)), make_quat(value_ptr(v2)), f)));
    };

    const string name(node->name);

    auto& take = _takes[_takeIndex];
    mat4 nodeTransform = node->transform;
    const auto res = findChannel(node->name);
    if (res) {
        // Interpolate and generate transformation matrix
        const auto& cha = (*res);
        const auto [t, r, s] = findFrame(time, cha);
        const vec3 tVec = vec3(getInterpolatedValue(time, t, cha.translations, vec3Interp));
        const vec4 rVec = getInterpolatedValue(time, t, cha.rotations, quatInterp);
        const quat rQuat = make_quat(value_ptr(rVec));
        const vec3 sVec = vec3(getInterpolatedValue(time, t, cha.scales, vec3Interp));

        const mat4 tMat = translate(mat4(1.0f), tVec);
        const mat4 rMat = toMat4(rQuat);
        const mat4 sMat = scale(mat4(1.0f), sVec);
#ifdef USE_FBX
		nodeTransform = tMat * mat4(mat3(node->transform)) * rMat * sMat; // 
#else
		nodeTransform = tMat * rMat * sMat;
#endif
    }

    mat4 world = parent * nodeTransform;

    if (_boneMap.find(name) != _boneMap.end()) {
        const uint32_t boneIndex = _boneMap[name];
		_boneInfo[boneIndex].worldMatrix = _globalInverseTransform * world *_boneInfo[boneIndex].bindingMatrix;
    }else {
		//Logger::getInstance()->info("Not a animation bone:" + name);
    }

    for (auto& child : node->children) {
        computeWorldMatrix(time, child.get(), world);
    }
}

void AnimatedMesh::processFBXAnimation(Take& seq) {
    std::vector<std::unique_ptr<Channel>> temp;
    std::unordered_set<std::string> processed;

    for (uint32_t i = 0; i < seq.channels.size(); i++) {
        auto& cha = seq.channels[i];
        auto strtok = split(cha->jointName, '$');
        if (strtok.size() > 1) {
            // complex animation, need to manually compose
            // Remove _ in jointName_
            auto jointName = strtok[0];
            jointName.pop_back();

            // if not yet compose,
            if (processed.find(jointName) == processed.end()) {
                temp.push_back(composeChannel(seq, jointName));
                processed.insert(jointName);
            }
        }
    }

    // Clear the original channels
    seq.channels.clear();

    // Copy back all composed new channels
    move(seq.channels, temp);

    // Create map based on name
    for (uint32_t i = 0; i < seq.channels.size(); i++) {
        auto cha = seq.channels[i].get();
        seq.channelMap.insert({cha->jointName, cha});
    }
}

std::unique_ptr<Channel> AnimatedMesh::composeChannel(Take& seq, std::string& jointName) {
    // Gather all corresponding channels
    Channel* translation = nullptr;
    Channel* rotation = nullptr;
    Channel* scaling = nullptr;

    for (uint32_t i = 0; i < seq.channels.size(); i++) {
        auto& cha = seq.channels[i];
        auto strtok = split(cha->jointName, '$');
        if (strtok.size() > 1) {
            // complex animation, need to manually compose
            // Remove _ in jointName_
            auto name = strtok[0];
            name.pop_back();

            // if this is the correct channel
            if (name == jointName) {
                // Check what kind of this animation it is.
                auto type = strtok[2];
                type.erase(0, 1);

                if (type == "Translation") {
                    translation = cha.get();
                }
                else if (type == "Rotation") {
                    rotation = cha.get();
                }
                else if (type == "Scaling") {
                    scaling = cha.get();
                }
                else {
                    Logger::getInstance()->warn("Unknown channel type:" + type);
                }
            }
        }
    }

    auto ret = std::make_unique<Channel>();
    ret->jointName = jointName;
    if (translation) {
        move(ret->translations, translation->translations);
    }

    if (rotation) {
        move(ret->rotations, rotation->rotations);
    }

    if (scaling) {
        move(ret->scales, scaling->scales);
    }

    return ret;
}

void AnimatedMesh::loadTakes(const std::string& filename) {
    // Replace extension
    // Find ext
    const auto extPos = filename.find_last_of('.');
    auto nameWithoutExt = filename;

    // Remove ext
    nameWithoutExt.erase(extPos);

    // Append ext
    const auto takeName = nameWithoutExt + TAKE_EXT;

    // Load Takes from file
    std::ifstream infile(takeName);
    std::string name;
    int start, end;
    while (infile >> name >> start >> end) {
        // Process takes info
        auto& t = _takeInfo.emplace_back();
        t.takeName = name;
        t.start = start;
        t.end = end;
    }

    // Process takes based on loaded in information
    std::vector<std::unique_ptr<Take>> temp;
    const auto& allTake = _takes[0];
    for (uint32_t i = 0; i < _takeInfo.size(); i++) {
        const auto& info = _takeInfo[i];
        auto newTake = std::make_unique<Take>();
        const auto startIndex = info.start;
        const auto endIndex = info.end;
        const auto size = endIndex - startIndex;

        // Copy metadata 
        newTake->takeName = info.takeName;
        newTake->tickrate = allTake->tickrate;

        // Create channels
        for (uint32_t j = 0; j < allTake->channels.size(); j ++) {
            auto cha = std::make_unique<Channel>();
            auto& from = allTake->channels[j];
            // Copy name
            cha->jointName = from->jointName;

            // Copy frame
            move(cha->translations, from->translations, startIndex, size);
            move(cha->rotations, from->rotations, startIndex, size);
            move(cha->scales, from->scales, startIndex, size);

            // Calculate offset
            const auto startTime = cha->translations[0]->time;

            // Offset keyframe
            for (uint32_t k = 0; k < cha->translations.size(); k++) {
                cha->translations[k]->time -= startTime;
                cha->rotations[k]->time -= startTime;
                cha->scales[k]->time -= startTime;
            }

            newTake->channels.push_back(std::move(cha));
        }

        // Build channel map
        for (uint32_t i = 0; i < newTake->channels.size(); i++) {
            auto cha = newTake->channels[i].get();
            newTake->channelMap.insert({cha->jointName, cha});
        }

        // Calculate duration
        auto& list = newTake->channels[0]->translations;
        const auto startTime = list[0]->time;
        const auto endTime = list[list.size() - 1]->time;
        newTake->duration = endTime - startTime;

        // Save the new take
        auto index = temp.size();
        _takeMap.insert({newTake->takeName, index});
        temp.push_back(std::move(newTake));
    }

    // Remove original 
    _takes.clear();

    // Copy temp into _takes
    move(_takes, temp);
}
