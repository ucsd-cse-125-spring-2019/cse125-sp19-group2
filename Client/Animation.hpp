#pragma once
#include "AnimatedMesh.hpp"
#include "Shader.hpp"

class Animation {
public:
	std::unique_ptr<AnimatedMesh> animatedMesh;

	bool isPlaying;
	float speed;
	static const uint32_t MAX_BONES = 100;
	GLuint m_boneLocation[MAX_BONES];
	std::vector<glm::mat4> transforms;

	float timer;
	float lastTime;
	float timeStep;

    Animation(const std::string& filename);

    void update();

    void eval();

	void render(const std::unique_ptr<Shader> & shader);

    void init(const std::unique_ptr<Shader> & shader);

    void setBoneUniform(uint32_t index, const glm::mat4& transform);
};
