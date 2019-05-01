#pragma once

#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Texture.hpp"
#include <glm/glm.hpp>
#include <map>
#include "Shader.hpp"
#include <functional>

#define MAX_BONES_PER_VERTEX 8

struct Node {
    std::string name;
    glm::mat4 transform{};
    const Node* parent{};
    std::vector<std::unique_ptr<Node>> children;
};

struct Keyframe {
    float time;
    glm::vec4 value;

    Keyframe(float t, glm::vec4 v): time(t), value(v) {
    }
};

struct Channel {
    std::string jointName;
    std::vector<std::unique_ptr<Keyframe>> translations;
    std::vector<std::unique_ptr<Keyframe>> rotations;
    std::vector<std::unique_ptr<Keyframe>> scales;
};

struct Sequence {
    float tickrate{};
    float duration{};
    std::unordered_map<std::string, std::unique_ptr<Channel>> channels;
};

struct BoneData {
    glm::mat4 bindingMatrix{};
    glm::mat4 worldMatrix{};

    BoneData() {
        bindingMatrix = glm::mat4(0.0f);
        worldMatrix = glm::mat4(0.0f);
    }
};

struct WeightData {
    uint32_t ids[MAX_BONES_PER_VERTEX]{};
    float weights[MAX_BONES_PER_VERTEX]{};

    WeightData() {
        reset();
    };

    void reset() {
        memset(ids, 0, MAX_BONES_PER_VERTEX * sizeof(uint32_t));
        memset(weights, 0, MAX_BONES_PER_VERTEX * sizeof(float));
    }

    void addWeight(uint32_t boneID, float weight);
};


struct MeshData {
	MeshData() {
		_numIndices = 0;
		_baseVertex = 0;
		_baseIndex = 0;
		_textureIndex = 0xFFFFFFFF;
	}

	unsigned int _numIndices;
	unsigned int _baseVertex;
	unsigned int _baseIndex;
	unsigned int _textureIndex;
};

enum VERTEXBUFFERTYPES {
	INDEX,
	POSITION,
	NORMAL,
	TEXCOORD,
	BONE,
	TYPECOUNT
};

class AnimatedMesh {
public:
    int _sequenceIndex;

    AnimatedMesh();

    ~AnimatedMesh();

    bool loadMesh(const std::string& filename);

    void render(const std::unique_ptr<Shader>& shader);

    void getTransform(float second, std::vector<glm::mat4>& transforms);

private:

    // Helper functions related to model loading
	void loadBones(uint32_t meshIndex, const aiMesh* pMesh, std::vector<WeightData>& bones);
    void initMesh(uint32_t meshIndex,
                  const aiMesh* paiMesh,
                  std::vector<glm::vec3>& positions,
                  std::vector<glm::vec3>& normals,
                  std::vector<glm::vec2>& texCoords,
                  std::vector<WeightData>& bones,
                  std::vector<unsigned int>& indices);
	bool initMaterials(const aiScene* scene, const std::string& filename);
	bool initScene(const aiScene* scene, const std::string& filename);
	std::unique_ptr<Channel> initChannel(const aiNodeAnim* node);
	std::unique_ptr<Node> initNode(const Node* parent, const aiNode* node);
    std::unique_ptr<Sequence> initSequence(const aiAnimation* animation);

    // Helper functions related to frame look up
    const Channel* findChannel(const std::string& nodeName);
	uint32_t findKeyframe(float time, const std::vector<std::unique_ptr<Keyframe>>& keyframes);
	std::tuple<uint32_t, uint32_t, uint32_t> findFrame(float time, const Channel& channel);

	glm::vec4 getInterpolatedValue(float time, uint32_t start, const std::vector<std::unique_ptr<Keyframe>>& keyframes,
		std::function<glm::vec4(float, glm::vec4, glm::vec4)> interpolateFunction);
	void computeWorldMatrix(float time, const Node* node, const glm::mat4& parent);

    uint32_t _VAO;
    uint32_t _buffers[TYPECOUNT]{};

	glm::mat4 _globalInverseTransform{};

	std::unique_ptr<Node> _root;

    std::vector<MeshData> _entries;
    std::vector<Texture*> _textures;
    std::vector<std::unique_ptr<Sequence>> _sequences;
	std::vector<BoneData> _boneInfo;
    std::map<std::string, uint32_t> _boneMap;

};
