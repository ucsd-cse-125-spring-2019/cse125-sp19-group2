#pragma once
#include <string>
#include <fmod/fmod.hpp>
#include "AudioManager.hpp"

class AudioClip {
public:
    std::string _path; // Key
    std::string _fileName;
    FMOD_MODE _flag;
    FMOD::Sound* _audio{};

    AudioClip(const std::string& path, bool loop = true, bool isPositional = false);
};
