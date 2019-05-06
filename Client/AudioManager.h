#pragma once

#include "fmod/fmod.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "Shared/Logger.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "AudioClip.h"
#include "AudioSource.h"

class AudioManager {
public:

    static AudioManager& getInstance() {
        static AudioManager audioManager;
        return audioManager;
    }

    AudioManager() {
        FMOD_RESULT result;

        result = System_Create(&_system); // Create the main system object.
        if (result != FMOD_OK) {
            Logger::getInstance()->warn(
                "FMOD error! " + std::to_string(result) + ": " + std::string(FMOD_ErrorString(result)));
        }

        result = _system->init(512, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0); // Initialize FMOD.
        if (result != FMOD_OK) {
            Logger::getInstance()->warn(
                "FMOD error! " + std::to_string(result) + ": " + std::string(FMOD_ErrorString(result)));
        }

        _system->setSpeakerPosition(FMOD_SPEAKER_FRONT_LEFT, -1.0f, 0.0f, true);
        _system->setSpeakerPosition(FMOD_SPEAKER_FRONT_RIGHT, 1.0f, 0.0f, true);
        _system->setDSPBufferSize(512, 2);

        // listener init
        _pos = glm::vec3(0, 0, 0);
        _forward = glm::vec3(0, 0, -1);
        _up = glm::vec3(0, 1, 0);

        // Set Listener's default position
        _system->set3DListenerAttributes(
            0, vec3ToFmod(_pos, _p), 0, 
			vec3ToFmod(_forward, _f),
            vec3ToFmod(_up, _u));

    }

    void setListenerPose(glm::vec3 pos, glm::vec3 forward) {
        _pos = pos;
		_forward = forward;
        // Set Listener's position
        _system->set3DListenerAttributes(
            0, vec3ToFmod(_pos, _p), 0, 
			vec3ToFmod(_forward, _f),
            vec3ToFmod(_up, _u));
    }

    static FMOD_VECTOR* vec3ToFmod(const glm::vec3& v, FMOD_VECTOR* fv) {
        fv->x = v.x;
        fv->y = v.y;
        fv->z = v.z;
        return fv;
    }

    // System
    FMOD::System* _system;

    // ambient
    FMOD::Sound* _ambientAudio;
    FMOD::Channel* _ambientChannel;

    // positional
    FMOD::Sound* _positionalAudio;
    FMOD::Channel* _positionalChannel;

    std::unordered_map<std::string, std::unique_ptr<AudioClip>> _audioClips;
    std::unordered_map<std::string, int> _audioSourceMap;
    std::vector<std::unique_ptr<AudioSource>> _audioSources;

private:    
    // Listener Vector
    glm::vec3 _pos;
    glm::vec3 _forward;
    glm::vec3 _up;
    FMOD_VECTOR* _p;
    FMOD_VECTOR* _f;
    FMOD_VECTOR* _u;
};
