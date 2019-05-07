#include "AudioClip.hpp"

using namespace std;

AudioClip::AudioClip(const string& path, bool loop, bool isPositional) {
    FMOD_MODE flag = 0;

    // Save path and filename
    const size_t found = path.find_last_of("/\\");
    _path = path;
    _fileName = path.substr(found + 1);

    // Create flag based on argument
    if (loop) {
        flag |= FMOD_LOOP_NORMAL;
    }

    if (isPositional) {
        flag |= (FMOD_3D | FMOD_3D_WORLDRELATIVE | FMOD_3D_INVERSEROLLOFF);
    }
    _flag = flag;

    // Create FMOD::Sound
    FMOD_RESULT res = AudioManager::getInstance().getSystem()->createSound(
        _path.c_str(),
        _flag,
        NULL,
        &_sound
    );

    if (res != FMOD_OK) {
        Logger::getInstance()->warn(
            "FMOD error! " + to_string(res) + ": " + string(FMOD_ErrorString(res)));
    }
}
