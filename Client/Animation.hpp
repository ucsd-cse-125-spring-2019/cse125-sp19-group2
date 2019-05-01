#pragma once
#include "AnimatedMesh.hpp"
#include "Shader.hpp"

class Animation {
public:
	std::unique_ptr<AnimatedMesh> animatedMesh;

    // TODO: see if Deacon can put different animation in different sequence or not
	std::unordered_map<std::string, std::pair<float, float>> states;
	bool isPlaying;
	float speed;
	static const uint32_t MAX_BONES = 100;
	GLuint m_boneLocation[MAX_BONES];
	std::vector<glm::mat4> transforms;

	float timer;
	float lastTime;
	float timeStep;

    Animation(const std::string& filename);

    void setStateDuration(std::initializer_list<std::pair<std::string, std::pair<float, float>>> state) {
        for (auto & element : state) {
			auto &[stateName, duration] = element;
			states.insert_or_assign(stateName, duration);
        }
    }

    void update(const std::string& state);

    void eval(const std::string& state);

	void render(const std::unique_ptr<Shader> & shader);

    void init(const std::unique_ptr<Shader> & shader);

    void setBoneUniform(uint32_t index, const glm::mat4& transform);
};
