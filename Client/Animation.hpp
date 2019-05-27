#pragma once
#include "Drawable.hpp"
#include "AnimatedMesh.hpp"
#include "Shader.hpp"

class AnimatedMesh;

struct TakeSequence {
    std::vector<std::string> sequence;
    std::vector<Take*> takesInSeq;
    std::vector<Transition> transitions;
    bool isPlaying = false;;
    float endTime = 0;
    float totalTime = 0;

    void addTake(std::string takename);

    void prepareSequence(AnimatedMesh* mesh, float totalTime);

    void clear() {
        takesInSeq.clear();
        transitions.clear();
    }
};

/**
 * \brief The class that handles playing animation
 */
class Animation : public Drawable {
public:
    std::unique_ptr<AnimatedMesh> _animatedMesh;

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
    void render(const std::unique_ptr<Shader>& shader) override;

    /**
     * \brief Initialize the related uniform for animation (specifically the boneMatrix Uniform)
     * \param shader(const std::unique_ptr<Shader>&) The shader program to render the mesh
     */
    void init(const std::unique_ptr<Shader>& shader);

    void play(std::string newTake, bool withTransition = true) {
        if (newTake != _currentTakeStr && !_isPlayOnce) {
            _lastTakeStr = _currentTakeStr;
            _currentTakeStr = newTake;
            _isTransition = true;
            //playTransition(_lastTakeStr, _currentTakeStr, _timer);
            /*if(withTransition){
                playTransition(_lastTakeStr, _currentTakeStr, _timer);
			}else {
			    _animatedMesh->setTakes(_currentTakeStr);
			}*/
            _timer = 0;
        }
    }

    void playOnce(std::string newTake, float endingTime, bool withTransition = true) {
        if (newTake != _currentTakeStr) {
            if(!_isPlayOnce){
                _lastTakeStr = _currentTakeStr;
			}
            _currentTakeStr = newTake;
            _isPlayOnce = true;
            _endingTime = endingTime;
            _isTransition = true;
            //playTransition(_lastTakeStr, _currentTakeStr, _timer);
            /*if(withTransition){
                playTransition(_lastTakeStr, _currentTakeStr, _timer);
			}else {
			    _animatedMesh->setTakes(_currentTakeStr);
			}*/
            _timer = 0;
        }
    }

    void playTransition(std::string from, std::string to, float time = -1) {
        transition = _animatedMesh->getTransition(from, to, time);
        playTransition(transition);
    }

    void playTransition(Transition & transition) {
        if (auto* pval = std::get_if<KeyframePair>(&transition)) {
            // TODO
            //_isTransition = true;
        }
        else if (auto* pval = std::get_if<Take*>(&transition)) {
            playOnce((*pval)->takeName, 0, false);
        }else {
            throw std::exception("Bad format in Transition");
        }
    }

    void playSequence(float totalTime) {
        if (!_isPlayOnce) {
            _isPlayingSequence = true;
            sequence->isPlaying = true;
            index = 0;
            sequence->prepareSequence(_animatedMesh.get(), totalTime);
            // seq
            auto t = sequence->takesInSeq[index / 2];
            _takeStrBeforeSeq = _currentTakeStr;
            playOnce(t->takeName, 0, false);
            index += 1;
        }
    }

    TakeSequence& getSequence() {
        return *sequence;
    }

private:
    void setBoneUniform(uint32_t index, const glm::mat4& transform);

    static const uint32_t MAX_BONES = 100;
    GLuint _boneLocation[MAX_BONES];
    std::vector<glm::mat4> _transforms{};

    float _timer; // in millisecond
    std::chrono::steady_clock::time_point _lastTime;

    std::string _lastTakeStr;
    std::string _takeStrBeforeSeq;
    std::string _currentTakeStr;

    // Value for transition
    bool _isTransition = false;

    // Value for playOnce
    bool _isPlayOnce = false;
    float _duration = -1;
    float _endingTime = -1; // in millisecond
    Transition transition;

    // 
    bool _isPlayingSequence = false;
    std::unique_ptr<TakeSequence> sequence;
    int index = 0;
};
