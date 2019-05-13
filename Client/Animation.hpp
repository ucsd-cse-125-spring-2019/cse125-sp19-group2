#pragma once
#include "Drawable.hpp"
#include "AnimatedMesh.hpp"
#include "Shader.hpp"

/**
 * \brief The class that handles playing animation
 */
class Animation : public Drawable {
public:
	std::unique_ptr<AnimatedMesh> animatedMesh;

    /**
	 * \brief The parameter to determine whether it should play animation or not
	 */
    bool _isPlaying;

    /**
	 * \brief The speed of the animation
	 */
    float _speed;

    /**
	 * \brief The timestep of the animation (in millisecond)
	 */
    float _timeStep;

    /**
     * \brief Constructor of Animation, load the animated mesh
     * \param filename(const std::string&) The path to the animated mesh
     */
    Animation(const std::string& filename);

    /**
     * \brief Being called each frame, to update the animation
     */
    void update() override;

    /**
     * \brief Being called by update, if isPlaying == true
     */
    void eval();

	/**
	 * \brief Render the mesh with texture and animation
	 * \param shader(const std::unique_ptr<Shader>&) The shader program to render the mesh
	 */
    void render(const std::unique_ptr<Shader> & shader) override;

    /**
     * \brief Initialize the related uniform for animation (specifically the boneMatrix Uniform)
     * \param shader(const std::unique_ptr<Shader>&) The shader program to render the mesh
     */
    void init(const std::unique_ptr<Shader> & shader);

private:
	void setBoneUniform(uint32_t index, const glm::mat4& transform);

	static const uint32_t MAX_BONES = 100;
	GLuint _boneLocation[MAX_BONES];
	std::vector<glm::mat4> _transforms{};

	float _timer; // in millisecond
	std::chrono::steady_clock::time_point _lastTime;
};
