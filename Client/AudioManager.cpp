#include "AudioManager.hpp"

using namespace std;
using namespace glm;

AudioManager& AudioManager::getInstance() {
    static AudioManager audioManager;
    return audioManager;
}

AudioManager::AudioManager() {
    FMOD_RESULT result;

    result = System_Create(&_system); // Create the main system object.
    if (result != FMOD_OK) {
        Logger::getInstance()->warn(
            "FMOD error! " + to_string(result) + ": " + string(FMOD_ErrorString(result)));
    }

    result = _system->init(512, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0); // Initialize FMOD.
    if (result != FMOD_OK) {
        Logger::getInstance()->warn(
            "FMOD error! " + to_string(result) + ": " + string(FMOD_ErrorString(result)));
    }

    _system->setSpeakerPosition(FMOD_SPEAKER_FRONT_LEFT, -1.0f, 0.0f, true);
    _system->setSpeakerPosition(FMOD_SPEAKER_FRONT_RIGHT, 1.0f, 0.0f, true);
    _system->setDSPBufferSize(512, 2);

    // listener init
    _pos = vec3(0, 0, 0);
    _forward = vec3(0, 0, -1);
    _up = vec3(0, 1, 0);

    FMOD_VECTOR p, f, u;

    // Set Listener's default position
    _system->set3DListenerAttributes(
        0, vec3ToFmod(_pos, &p), 0,
        vec3ToFmod(_forward, &f),
        vec3ToFmod(_up, &u));

}

void AudioManager::setListenerPose(vec3 pos, vec3 forward) {
    _pos = pos;
    _forward = forward;
    FMOD_VECTOR p, f, u;
    // Set Listener's position
    _system->set3DListenerAttributes(
        0, vec3ToFmod(_pos, &p), 0,
        vec3ToFmod(_forward, &f),
        vec3ToFmod(_up, &u));
}

FMOD_VECTOR* AudioManager::vec3ToFmod(const vec3& v, FMOD_VECTOR* fv) {
    fv->x = v.x;
    fv->y = v.y;
    fv->z = v.z;
    return fv;
}

AudioSource* AudioManager::getAudioSource(const std::string& name, vec3 pos) {
    const auto res = _audioSourceMap.find(name);
    if (res != _audioSourceMap.end()) {
        return _audioSources[res->second].get();
    }
    auto source = std::make_unique<AudioSource>();
    source->_name = name;
    source->setPosition(pos);
    auto* ret = _audioSources.emplace_back(move(source)).get();
    _audioSourceMap.insert({name, _audioSources.size() - 1});
    return ret;
}

AudioClip* AudioManager::getAudioClip(const string& path, bool loop, bool isPositional) {
    const auto res = _audioClips.find(path);
    if (res != _audioClips.end()) {
        return res->second.get();
    }
    auto clip = make_unique<AudioClip>(path, loop, isPositional);
    auto* ret = _audioClips.insert({path, move(clip)}).first->second.get();
    return ret;
}

void AudioManager::update() {
    _system->update();
}

FMOD::System* AudioManager::getSystem() const {
    return _system;
}
