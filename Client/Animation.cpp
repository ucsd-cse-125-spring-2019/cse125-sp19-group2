#include "Animation.hpp"

using namespace std;
using namespace glm;
using namespace chrono;

Animation::Animation(const string& filename): _isPlaying(false), _speed(1.0f), _timer(0.0f), _timeStep(-1.0f) {
    _animatedMesh = make_unique<AnimatedMesh>();
    _animatedMesh->loadMesh(filename);
}

void Animation::update() {
    if (_isPlaying) {
        eval();
    }
}

void Animation::eval() {
    if (_isTransition) {
        const float currentTime = _animatedMesh->getTimeInTick(_lastTake, _timer / 1000.0f);
        // TODO

        if (currentTime > -1) {
            _animatedMesh->_takeIndex = _currentTake;
            _isTransition = false;
            _timer = 0;
        }
    }
    else {
        if (_lastTime.time_since_epoch().count() == 0) {
            _lastTime = high_resolution_clock::now();
            return;
        }

        const auto curr = high_resolution_clock::now();
        const auto dt = curr - _lastTime;
        const float dtInNanosecond = chrono::duration_cast<nanoseconds>(dt).count();

        if (_timeStep < 0) {
            _timer += (dtInNanosecond / 1000000.0) * _speed;
        }
        else {
            _timer += _timeStep;
        }

        // PlayOnce logic
        if (_isPlayOnce) {
            // check if the animation is finished
            const float currentTime = _animatedMesh->getTimeInTick(_currentTake, _timer / 1000.0f);
            const float endingTime = _animatedMesh->getTimeInTick(_currentTake, _endingTime / 1000.0f);
            const float duration = _animatedMesh->getDuration(_currentTake);
            if (currentTime > duration) {
                if(currentTime > duration + endingTime){
                    // the animation is finished, switch back to the original one
                    _isPlayOnce = false;
                    play(_lastTake);
                    _lastTime = high_resolution_clock::now();
                    return;
				}else {
                    _lastTime = high_resolution_clock::now();
				    return;
				}
            }
        }

        _animatedMesh->getTransform(_timer / 1000.0f, _transforms);
        _lastTime = high_resolution_clock::now();
    }
}

void Animation::render(const unique_ptr<Shader>& shader) {
    shader->Use();

    for (int i = 0; i < _transforms.size(); i ++) {
        setBoneUniform(i, _transforms[i]);
    }

    _animatedMesh->render(shader);
}

void Animation::init(const unique_ptr<Shader>& shader) {
    shader->Use();
    for (unsigned int i = 0; i < MAX_BONES; i++) {
        char name[128];
        memset(name, 0, sizeof(name));
        snprintf(name, sizeof(name), "u_bones[%d]", i);
        _boneLocation[i] = glGetUniformLocation(shader->program(), name);
    }
}

void Animation::setBoneUniform(uint32_t index, const mat4& transform) {
    glUniformMatrix4fv(_boneLocation[index], 1, GL_FALSE, &transform[0][0]);
}
