#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fmod/fmod.hpp>
#include "AudioManager.hpp"

// Forward Declaration
class AudioClip;

class AudioSource {
public:
    std::string _name;
    glm::vec3 _pos;
    glm::vec3 _vel;
    bool _isPlaying = false;
    float _volume = 1.0;
    FMOD::Channel* _channel{};
    AudioClip* _currentClip;

    AudioSource();
    AudioSource(AudioClip* clip);

    void init(const std::string & path, bool loop = true, bool isPositional = false);

    void setPosition(glm::vec3 pos);

    void setVelocity(glm::vec3 vel);

    void setVolume(float vol);

    void play(bool playing = true);

private:
    void updateChannel3DAttribute();
};
