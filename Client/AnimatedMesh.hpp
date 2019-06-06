#pragma once

#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "Texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include "Shader.hpp"
#include <functional>
#include <sstream>
#include "Shared/Logger.hpp"
#include <optional>
#include <variant>

#define MAX_BONES_PER_VERTEX 8
#define TAKE_EXT ".txt"

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
    int start{};
    int end{};
};

struct Node {
    std::string name;
    glm::mat4 transform{};
    const Node* parent{};
    std::vector<std::shared_ptr<Node>> children;
};

struct Keyframe {
    float time;
    glm::vec4 translate;
    glm::quat rotation;
    glm::vec4 scale;
    Keyframe();
    Keyframe(float time, glm::vec4 t, glm::quat r, glm::vec4 s) :
        time(time), translate(t), rotation(r), scale(s)
    {
    }

    static Keyframe interpolatingFuntion(float factor, const Keyframe& from, const Keyframe& to);
};

struct KeyframeAll {
    float time;
    int index = -1;
    std::vector<Keyframe*> channels;
};

struct Channel {
    std::string jointName;
    std::vector<std::shared_ptr<Keyframe>> keyframes;
};

struct Take {
    std::string takeName;
    float tickrate{};
    float duration{};
    std::vector<std::shared_ptr<Channel>> channels;
    std::unordered_map<std::string, uint32_t> channelMap;
};

typedef std::tuple<KeyframeAll, KeyframeAll, float> KeyframePair;
typedef std::variant<KeyframePair, Take*> Transition;

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

/**
 * \brief A class that handles animationMesh loading and animation look up.
 */
class AnimatedMesh {
public:

    AnimatedMesh();

    ~AnimatedMesh();

    /**
     * \brief Load the animated mesh
     * \param filename(const std::string&) Path to the animated mesh
     * \return bool: whether the loading is success
     */
    bool loadMesh(const std::string& filename);

    /**
     * \brief Render the mesh with texture
     * \param shader(const std::unique_ptr<Shader>&) The shader program to render the mesh
     */
    void render(const std::unique_ptr<Shader>& shader);

    /**
     * \brief Lookup the animation transforms at the given time (in second), and populate output std::vector
     * \param second(float) Time (in seconds)
     * \param transforms(std::vector<glm::mat4>&) Output parameter to saved the transforms
     */
    void getTransform(float second, std::vector<glm::mat4>& transforms);

    void setTakes(std::string name);

    /**
     * \brief Getter, return the animation count of this animated mesh
     * \return uint32_t: The animation count of this animated mesh
     */
    uint32_t takeCount() const;

    float getDuration(std::string name);

    float getTimeInTick(std::string name, float second);
    float getTimeInTick(int takeIndex, float second);

    std::string getCurrentAnimName() const;

    std::optional<uint32_t> getTakeIndex(std::string name);

    Take* getTake(uint32_t index);

    KeyframeAll getKeyframeAlongChannel(std::string takeName, float time);
    KeyframeAll getKeyframeAlongChannel(int takeIndex, float time);
    KeyframeAll getKeyframeAlongChannelByIndex(int takeIndex, int index);

    Transition getTransition(std::string from, std::string to, float time = -1.0);

private:

    // Helper functions related to model loading
    void loadBones(uint32_t meshIndex, const aiMesh* pMesh, std::vector<WeightData>& bones);

    void initMesh(
        uint32_t meshIndex,
        const aiMesh* paiMesh,
        std::vector<glm::vec3>& positions,
        std::vector<glm::vec3>& normals,
        std::vector<glm::vec2>& texCoords,
        std::vector<WeightData>& bones,
        std::vector<unsigned int>& indices);

    bool initMaterials(const aiScene* scene, const std::string& filename);

    bool initScene(const aiScene* scene, const std::string& filename);

    std::shared_ptr<Channel> initChannel(const aiNodeAnim* node);

    std::shared_ptr<Node> initNode(const Node* parent, const aiNode* node);

    std::shared_ptr<Take> initTake(const aiAnimation* animation);

    // Helper functions related to frame look up
    const Channel* findChannel(const std::string& nodeName);

    int findChannelIndex(const std::string& nodeName);

    uint32_t findFrame(float time, const Channel& channel);

    Keyframe getInterpolatedValue(float time, uint32_t start,
                                        const std::vector<std::shared_ptr<Keyframe>>& keyframes,
                                        std::function<Keyframe(float, const Keyframe&, const Keyframe&)> interpolateFunction);

    bool computeWorldMatrix(float time, const Node* node, const glm::mat4& parent);

    void computeWorldMatrix(float time, KeyframeAll & from, KeyframeAll & to, const Node* node, const glm::mat4& parent);

    template <typename T>
    void move(std::vector<T>& to, std::vector<T>& from);

    template <typename T>
    void move(std::vector<T>& to, std::vector<T>& from, size_t start, size_t size);

    template <typename T, class Iterator, class I = typename std::iterator_traits<Iterator>::value_type>
    void move(std::vector<T>& to, I& fromStart, I& fromEnd);

    void loadTakes(const std::string& filename);

    std::tuple<int, uint32_t> getKeyFrame(int takeIndex, float time) const {

    }

    uint32_t _VAO;
    uint32_t _buffers[TYPECOUNT]{};

    glm::mat4 _globalInverseTransform{};

    std::shared_ptr<Node> _root;
    
    int _takeIndex;
    std::vector<MeshData> _entries;
    std::vector<Texture*> _textures;
    std::vector<std::shared_ptr<Take>> _takes;
    std::vector<BoneData> _boneInfo;
    std::map<std::string, uint32_t> _boneMap;
    std::map<std::string, uint32_t> _takeMap;
    std::vector<TakeInfo> _takeInfo;
    std::vector<Node*> _nodeList;
};

template <typename T>
void AnimatedMesh::move(std::vector<T>& to, std::vector<T>& from) {
    for (uint32_t i = 0; i < from.size(); i ++) {
        to.push_back((from[i]));
    }
}

template <typename T>
void AnimatedMesh::move(std::vector<T>& to, std::vector<T>& from, size_t start, size_t size) {
    for (uint32_t i = start; i < size + start && i < from.size(); i++) {
        to.push_back((from[i]));
    }
}

template <typename T, class Iterator, class I>
void AnimatedMesh::move(std::vector<T>& to, I& fromStart, I& fromEnd) {
    for (auto it = fromStart; it < fromEnd; ++it) {
        to.push_back((*it));
    }
}
