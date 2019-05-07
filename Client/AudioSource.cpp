#include "AudioSource.hpp"

using namespace std;
using namespace glm;

AudioSource::AudioSource() {
    _pos = vec3(0, 0, 0);
    _vel = vec3(0, 0, 0);
}

AudioSource::AudioSource(AudioClip* clip) {
    _pos = vec3(0, 0, 0);
    _vel = vec3(0, 0, 0);
    _currentClip = clip;

    FMOD_RESULT res = AudioManager::getInstance().getSystem()->playSound(
        _currentClip->_sound,
        NULL,
        !_isPlaying,
        &_channel
    );

    if (res != FMOD_OK) {
        Logger::getInstance()->warn(
            "FMOD error! " + to_string(res) + ": " + string(FMOD_ErrorString(res)));
    }
}

void AudioSource::init(const string& path, bool loop, bool isPositional) {
    auto clip = AudioManager::getInstance().getAudioClip(path, loop, isPositional);

    _currentClip = clip;

    FMOD_RESULT res = AudioManager::getInstance().getSystem()->playSound(
        _currentClip->_sound,
        NULL,
        !_isPlaying,
        &_channel
    );

    if (res != FMOD_OK) {
        Logger::getInstance()->warn(
            "FMOD error! " + to_string(res) + ": " + string(FMOD_ErrorString(res)));
    }
}

void AudioSource::setPosition(vec3 pos) {
    _pos = pos;
    updateChannel3DAttribute();
}

void AudioSource::setVelocity(vec3 vel) {
    _vel = vel;
    updateChannel3DAttribute();
}

void AudioSource::setVolume(float vol) {
    _volume = vol;
    _channel->setVolume(_volume);
}

void AudioSource::play(bool playing) {
    _isPlaying = playing;
    _channel->setPaused(!_isPlaying);
}

void AudioSource::updateChannel3DAttribute() {
    FMOD_VECTOR p, v;
    _channel->set3DAttributes(
        AudioManager::vec3ToFmod(_pos, &p),
        AudioManager::vec3ToFmod(_vel, &v)
    );
}
