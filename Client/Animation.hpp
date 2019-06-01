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

    void play(std::string newTake, bool noTransition = false) {
        if (newTake != _currentTakeStr && !_isPlayOnce) {
            //if(noTransition){
            //    std::cout << "NoTran Play:" << newTake << std::endl;
			//}else {
            //    if(newTake == "swinging1") {
            //        //std::cout << "bp" << std::endl;
            //    }
			//    std::cout << "Tran Play:" << newTake << std::endl;
			//}
            _lastTakeStr = _currentTakeStr;
            if(!noTransition) {
                //_takeBeforeTransitionStr = _lastTakeStr;
            }
            _currentTakeStr = newTake;
            _isTransition = true;
            if(!noTransition){
                _takeAfterTransitionStr = newTake;
                playTransition(_lastTakeStr, _takeAfterTransitionStr);
			}
        }
    }

    void playOnce(std::string newTake, float endingTime, bool noTransition = false) {
        if (newTake != _currentTakeStr && !_isPlayOnce) {
            //if(noTransition){
            //    std::cout << "NoTran Playonce:" << newTake << std::endl;
			//}else {
            //    if(newTake == "swinging1") {
            //        //std::cout << "bp" << std::endl;
            //    }
			//    std::cout << "Tran Playonce:" << newTake << std::endl;
			//}
            _lastTakeStr = _currentTakeStr;
            if(!noTransition) {
                _takeBeforeTransitionStr = _lastTakeStr;
                _isPlayOnceAfter = true;
            }
            _currentTakeStr = newTake;
            _isPlayOnce = true;
			if (endingTime > 0) {
				_endingTime = endingTime;
			}
			else {
				_endingTime = 0;
			}
            _isTransition = true;
			if(!noTransition){
                _takeAfterTransitionStr = newTake;
				_endingTime -= _animatedMesh->getDuration(newTake) * 1000;
                playTransition(_lastTakeStr, _takeAfterTransitionStr);
			}
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
            _isPlayOnceAfter = false;
        }
        else if (auto* pval = std::get_if<Take*>(&transition)) {
            _isPlayingTransition = true;
            playOnce((*pval)->takeName, -1, true);
			_endingTime -= _animatedMesh->getDuration((*pval)->takeName) * 1000;
        }else {
            throw std::exception("Bad format in Transition");
        }
    }

    void playSequence(float totalTime) {
        //if (!_isPlayOnce) {
        //    _isPlayingSequence = true;
        //    sequence->isPlaying = true;
        //    index = 0;
        //    sequence->prepareSequence(_animatedMesh.get(), totalTime);
        //    // seq
        //    auto t = sequence->takesInSeq[index / 2];
        //    _takeStrBeforeSeq = _currentTakeStr;
        //    playOnce(t->takeName, 0, false);
        //    index += 1;
        //}
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
    std::string _takeBeforeTransitionStr;
    std::string _takeAfterTransitionStr;
    std::string _currentTakeStr;

    // Value for transition
    bool _isTransition = false;
    bool _isPlayingTransition = false;

    // Value for playOnce
    bool _isPlayOnce = false;

    bool _isPlayOnceAfter = false;
    float delay = -1;

    float _duration = -1;
    float _endingTime = -1; // in millisecond
    Transition transition;

    // 
    bool _isPlayingSequence = false;
    std::unique_ptr<TakeSequence> sequence;
    int index = 0;
};
