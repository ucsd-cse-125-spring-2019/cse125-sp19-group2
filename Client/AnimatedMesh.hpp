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
#include <sstream>
#include <fstream>
#include <unordered_set>
#include "Shared/Logger.hpp"

#define MAX_BONES_PER_VERTEX 8
#define TAKE_EXT ".take"

inline std::vector<std::string> split(std::string str, char delimiter) {
    std::vector<std::string> internal;
    std::stringstream ss(str); // Turn the string into a stream.
    std::string tok;

    while (getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }

    return internal;
}

struct TakeInfo {
    std::string takeName;
    int start;
    int end;
};

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

struct Take {
    std::string takeName;
    float tickrate{};
    float duration{};
    std::vector<std::unique_ptr<Channel>> channels;
    std::unordered_map<std::string, Channel*> channelMap;
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
    int _takeIndex;

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
    std::unique_ptr<Take> initTake(const aiAnimation* animation);

    // Helper functions related to frame look up
    const Channel* findChannel(const std::string& nodeName);
    uint32_t findKeyframe(float time, const std::vector<std::unique_ptr<Keyframe>>& keyframes);
    std::tuple<uint32_t, uint32_t, uint32_t> findFrame(float time, const Channel& channel);

    glm::vec4 getInterpolatedValue(float time, uint32_t start, const std::vector<std::unique_ptr<Keyframe>>& keyframes,
                                   std::function<glm::vec4(float, glm::vec4, glm::vec4)> interpolateFunction);
    void computeWorldMatrix(float time, const Node* node, const glm::mat4& parent);

    void processFBXAnimation(Take& seq) {
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

    template <typename T>
    void move(std::vector<T>& to, std::vector<T>& from) {
        for (uint32_t i = 0; i < from.size(); i ++) {
            to.push_back(std::move(from[i]));
        }
    }

	template <typename T>
	void move(std::vector<T>& to, std::vector<T>& from, size_t start, size_t size) {
		for (uint32_t i = start; i < size + start && i < from.size(); i++) {
			to.push_back(std::move(from[i]));
		}
	}

    template <typename T, class Iterator, class I = typename std::iterator_traits<Iterator>::value_type>
    void move(std::vector<T>& to, I& fromStart, I& fromEnd) {
        for (auto it = fromStart; it < fromEnd; ++it) {
            to.push_back(std::move(*it));
        }
    }

    std::unique_ptr<Channel> composeChannel(Take& seq, std::string& jointName) {
        // Gather all corresponding channels
        Channel* translation = nullptr;
        Channel* rotation = nullptr;
        Channel* scaling = nullptr;

        for (uint32_t i = 0; i < seq.channels.size(); i ++) {
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

    void loadTakes(const std::string& filename) {
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
		const auto & allTake = _takes[0];
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
				auto &from = allTake->channels[j];
                // Copy name
				cha->jointName = from->jointName;

                // Copy frame
				move(cha->translations, from->translations, startIndex, size);
				move(cha->rotations, from->rotations, startIndex, size);
				move(cha->scales, from->scales, startIndex, size);

				newTake->channels.push_back(std::move(cha));
            }

			// Build channel map
			for (uint32_t i = 0; i < newTake->channels.size(); i++) {
				auto cha = newTake->channels[i].get();
				newTake->channelMap.insert({ cha->jointName, cha });
			}

            // Calculate duration
			auto & list = newTake->channels[0]->translations;
            const auto startTime = list[0]->time;
            const auto endTime = list[list.size()-1]->time;
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

    uint32_t _VAO;
    uint32_t _buffers[TYPECOUNT]{};

    glm::mat4 _globalInverseTransform{};

    std::unique_ptr<Node> _root;

    std::vector<MeshData> _entries;
    std::vector<Texture*> _textures;
    std::vector<std::unique_ptr<Take>> _takes;
    std::vector<BoneData> _boneInfo;
    std::map<std::string, uint32_t> _boneMap;
    std::map<std::string, uint32_t> _takeMap;
    std::vector<TakeInfo> _takeInfo;
    std::vector<Node*> _nodeList;
};
