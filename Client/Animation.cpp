#include "Animation.hpp"
#include "InputManager.h"

using namespace std;
using namespace glm;
using namespace chrono;

void TakeSequence::addTake(std::string takename) {
    if(!isPlaying){
        sequence.push_back(takename);
	}
}

void TakeSequence::prepareSequence(AnimatedMesh* mesh, float totalTime) {
    takesInSeq.clear();
    transitions.clear();
    endTime = 0;
    this->totalTime = totalTime;

    // Populate takes
    float time = 0;
    int generated = 0;
    for (int i = 0; i < sequence.size(); i ++) {
        auto name = sequence[i];
        auto res = mesh->getTakeIndex(name);

        // Animation itself
        if (!res.has_value()) {
            throw std::exception("Non-exist animation name");
        }

        Take* t = mesh->getTake(res.value());
        time += t->duration;
        takesInSeq.push_back(t);

        // Find transition
        if (i != sequence.size() - 1) {
            auto from = sequence[i];
            auto to = sequence[i + 1];
            auto k = mesh->getTransition(from, to, -1);

            if (auto* pval = std::get_if<Take*>(&k)) {
                // is a Take pointer
                time += (*pval)->duration;
            }
            else {
                generated += 1;
            }
            transitions.push_back(k);
        }
    }

    // Sanitize time
    float diff = totalTime - time;
    if (diff < 0) {
        throw std::exception("Sequence time longer than total time");
    }

    // Distribute difference to generated transition
    if (diff > 0) {
        if (generated > 0) {
            float time = diff / float(generated);
            for (int i = 0; i < transitions.size(); i ++) {
                auto k = transitions[i];
                if (auto* pval = std::get_if<KeyframePair>(&k)) {
                    // is a KeyframePair
                    auto [from, to, t] = *pval;
                    transitions[i] = make_tuple(from, to, time);
                }
            }
        }
        else {
            // pad ending
            endTime = diff;
        }
    }

    sequence.clear();
}

Animation::Animation(const string& filename): _isPlaying(false), _speed(1.0f), _timeStep(-1.0f), _timer(0.0f),
                                              _currentTakeStr("idle"), _lastTakeStr("idle") {
    _animatedMesh = make_unique<AnimatedMesh>();
    _animatedMesh->loadMesh(filename);
    sequence = make_unique<TakeSequence>();

    //// Test event
    //InputManager::getInstance().getKey(GLFW_KEY_E)->onPress(
    //    [&] {
    //        sequence->addTake("running");
    //        sequence->addTake("swinging2");
    //        sequence->addTake("shoot");
    //        sequence->addTake("flying");
    //        sequence->addTake("slipping");
    //        sequence->addTake("swinging3");
    //        sequence->addTake("placing");
    //        this->playSequence(2000);
    //    });

    //// Test event
    //InputManager::getInstance().getKey(GLFW_KEY_K)->onPress(
    //    [&] {
    //        this->playOnce("swinging1-idle", 0);
    //    });
}

void Animation::update() {
    if (_isPlaying) {
        eval();
    }
}

void Animation::eval() {
    if (_isTransition) {
        if(_isPlayingTransition) {
            _animatedMesh->setTakes(_currentTakeStr);
        }else{
            _animatedMesh->setTakes(_takeAfterTransitionStr);
		}
        _isTransition = false;
        _timer = 0;
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
            const float currentTime = _animatedMesh->getTimeInTick(_currentTakeStr, _timer / 1000.0f);
            const float endingTime = _animatedMesh->getTimeInTick(_currentTakeStr, _endingTime / 1000.0f);
            const float duration = _animatedMesh->getDuration(_currentTakeStr);
            
            std::cout << "currentTime: " << currentTime << ";duration: " << duration << ";endingTime: " << endingTime << std::endl;
            if (currentTime > duration) {
                if (currentTime > duration + endingTime) {
                    _isPlayOnce = false;
                    if(_isPlayingTransition) {
                        if(_isPlayOnceAfter) {
                            playOnce(_takeAfterTransitionStr, -1, true);
                            _isPlayOnceAfter = false;
                        }else {
                            play(_takeAfterTransitionStr, true);
                        }
                        _isPlayingTransition = false;
                    }else{
                        play(_takeBeforeTransitionStr);
					}
                    _lastTime = high_resolution_clock::now();
                    return;
                }
                else {
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
