#pragma once

#include "fmod/fmod.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "Shared/Logger.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "AudioClip.hpp"
#include "AudioSource.hpp"

// Forward Declaration
class AudioClip;
class AudioSource;

class AudioManager {
public:

    static AudioManager& getInstance();

    AudioManager();

	bool getMute();

	void setMute(bool mute);

    void setListenerPos(glm::vec3 pos);

	void setListenerDir(glm::vec3 forward);

    static FMOD_VECTOR* vec3ToFmod(const glm::vec3& v, FMOD_VECTOR* fv);

    AudioSource* getAudioSource(const std::string& name, glm::vec3 pos = glm::vec3(0, 0, 0));

    AudioClip* getAudioClip(const std::string& path, bool loop = true, bool isPositional = false);

    void update();

    FMOD::System* getSystem() const;

	// Stops all sound clips and sources
	void reset();

private:
    // System
    FMOD::System* _system;

    // Listener Vector
    glm::vec3 _pos;
    glm::vec3 _forward;
    glm::vec3 _up;

    // Audio list
    std::unordered_map<std::string, std::unique_ptr<AudioClip>> _audioClips;
    std::unordered_map<std::string, int> _audioSourceMap;
    std::vector<std::unique_ptr<AudioSource>> _audioSources;
};
