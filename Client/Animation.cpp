#include "Animation.hpp"

Animation::Animation(const std::string& filename): isPlaying(false), speed(1.0f), lastTime(-1.0f), timer(0.0f), timeStep(8.33f) {
    animatedMesh = std::make_unique<AnimatedMesh>();
    animatedMesh->loadMesh(filename);
}

void Animation::update() {
    if (isPlaying) {
		eval();
    }
}

void Animation::eval() {
    if(lastTime < 0) {
		lastTime = static_cast<float>(GetTickCount());
		return;
    }
    const float currentTime = static_cast<float>(GetTickCount());
	if (timeStep < 0) {
		timer += (currentTime - lastTime) * speed;
	}else {
		timer += timeStep;
	}
	animatedMesh->getTransform(timer/1000.0f, transforms);
    lastTime = static_cast<float>(GetTickCount());
}

void Animation::render(const std::unique_ptr<Shader> & shader) {
	shader->Use();
    
    for (int i = 0; i < transforms.size(); i ++) {
        setBoneUniform(i, transforms[i]);
    }

    animatedMesh->render(shader);
}

void Animation::init(const std::unique_ptr<Shader> & shader) {
	shader->Use();
    for (unsigned int i = 0; i < MAX_BONES; i++) {
        char name[128];
        memset(name, 0, sizeof(name));
        snprintf(name, sizeof(name), "u_bones[%d]", i);
        m_boneLocation[i] = glGetUniformLocation(shader->program(), name);
    }
}

void Animation::setBoneUniform(uint32_t index, const glm::mat4& transform) {
    glUniformMatrix4fv(m_boneLocation[index], 1, GL_FALSE, &transform[0][0]);
}
